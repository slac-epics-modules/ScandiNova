#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <iocsh.h>
#include <epicsTypes.h>
#include <aSubRecord.h>
#include <registryFunction.h>
#include <epicsExport.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>
#include <map>
#include <utility>
#include <string>
#include "asynPortDriver.h"

#define EVENT_LOG_SIZE 50
#define EPOCH_RAW_TO_EPOCH(raw) ((raw) / 10000000 - 11644473600)
#define EPOCH_RAW_TO_MILLISECONDS_OFFSET(raw) (((raw) % 10000000) / 10000)

#define WAVEFORM_COUNT_MAX 40
#define WAVEFORM_SAMPLE_COUNT_MAX 503
#define WAVEFORM_READY_CODE_TO_REQUEST_CODE(code) ((waveform_code_t) ((code) - 100))


using namespace std;

typedef enum {
	EVENT_TYPE_STATE,
	EVENT_TYPE_WARNING,
	EVENT_TYPE_INTERLOCK,
	EVENT_TYPE_ERROR,
	EVENT_TYPE_PARAM,
	EVENT_TYPE_MESSAGE
} event_type_t;

typedef enum {
	WAVEFORM_UNKNOWN = -1,
	WAVEFORM_CVD_T0_REQUEST = 0,
	WAVEFORM_CVD_T1_REQUEST = 1,
	WAVEFORM_CVD_T2_REQUEST = 2,
	WAVEFORM_CVD_T3_REQUEST = 3,
	WAVEFORM_CVD_T4_REQUEST = 4,
	WAVEFORM_CT_T0_REQUEST = 10,
	WAVEFORM_CT_T1_REQUEST = 11,
	WAVEFORM_CT_T2_REQUEST = 12,
	WAVEFORM_CT_T3_REQUEST = 13,
	WAVEFORM_CT_T4_REQUEST = 14,
	WAVEFORM_RF_FWD_T0_REQUEST = 20,
	WAVEFORM_RF_FWD_T1_REQUEST = 21,
	WAVEFORM_RF_FWD_T2_REQUEST = 22,
	WAVEFORM_RF_FWD_T3_REQUEST = 23,
	WAVEFORM_RF_FWD_T4_REQUEST = 24,
	WAVEFORM_RF_RFL_T0_REQUEST = 30,
	WAVEFORM_RF_RFL_T1_REQUEST = 31,
	WAVEFORM_RF_RFL_T2_REQUEST = 32,
	WAVEFORM_RF_RFL_T3_REQUEST = 33,
	WAVEFORM_RF_RFL_T4_REQUEST = 34,
	WAVEFORM_FETCHING = 99,
	WAVEFORM_CVD_T0_READY = 100,
	WAVEFORM_CVD_T1_READY = 101,
	WAVEFORM_CVD_T2_READY = 102,
	WAVEFORM_CVD_T3_READY = 103,
	WAVEFORM_CVD_T4_READY = 104,
	WAVEFORM_CT_T0_READY = 110,
	WAVEFORM_CT_T1_READY = 111,
	WAVEFORM_CT_T2_READY = 112,
	WAVEFORM_CT_T3_READY = 113,
	WAVEFORM_CT_T4_READY = 114,
	WAVEFORM_RF_FWD_T0_READY = 120,
	WAVEFORM_RF_FWD_T1_READY = 121,
	WAVEFORM_RF_FWD_T2_READY = 122,
	WAVEFORM_RF_FWD_T3_READY = 123,
	WAVEFORM_RF_FWD_T4_READY = 124,
	WAVEFORM_RF_RFL_T0_READY = 130,
	WAVEFORM_RF_RFL_T1_READY = 131,
	WAVEFORM_RF_RFL_T2_READY = 132,
	WAVEFORM_RF_RFL_T3_READY = 133,
	WAVEFORM_RF_RFL_T4_READY = 134,
} waveform_code_t;

typedef struct __attribute__ (packed) {
	uint16_t increment;
	uint16_t type;
	uint64_t epoch_raw;
	uint32_t trigger;
	uint16_t matrix_index;
	uint16_t text_number;
	uint16_t data_type;
	uint32_t data;
} event_struct_t;

typedef struct {
	int increment;          // unique id (0, 1, 2, ..., 65535) for the event per boot
	time_t epoch;           // seconds since 1/1/1970
	char timestamp[64];     // human-readable timestamp derived from the modulator's time
	char timestamp_ioc[64]; // human-readable timestamp derived from the IOC's UNIX epoch
	int trigger;            // counter
	int type;               // index for Strings and event types
	char type_str[32];      // type description
	int text_number;        // index for Strings[type]
	char text_str[64];      // from Strings[type][text_number]
	char display_str[128];  // we show on the GUI
	int matrix_index;       // index for MatrixItems
	char subsystem_str[32]; // from MatrixItems[matrix_index].Name
	char units_str[8];      // from MatrixItems[matrix_index].Unit
	int data_type;          // index for data types; defines the conversion
	char data_type_str[16]; // data type description
	uint32_t data;          // raw data (4 bytes)
	char data_str[32];      // converted data to be read by humans
} event_info_t;


static int debug_flag = 0;
static pthread_t log_thread;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct timespec next_clean_time = {-1, -1};
static event_info_t event_infos[EVENT_LOG_SIZE];
static int current_event_index = -1;
static char *event_output_file = NULL;
static FILE *event_output_fd = NULL;
static waveform_code_t prev_waveform_ready_code = WAVEFORM_UNKNOWN;
static double waveforms[WAVEFORM_COUNT_MAX][WAVEFORM_SAMPLE_COUNT_MAX];
static char waveform_timestamp[WAVEFORM_COUNT_MAX][64];
static int waveform_trig_ids[WAVEFORM_COUNT_MAX];
static int waveform_sample_counts[WAVEFORM_COUNT_MAX];
static uint64_t waveform_request_enable = 0;

static char log_html_doc_template_start[] =                   \
	"<!DOCTYPE html>\n"                                       \
	"<html>\n"                                                \
	"<head>\n"                                                \
	" <meta http-equiv=\"refresh\" content=\"1\">\n"          \
    "</head>\n"                                               \
	" <body>\n"                                               \
	"  <table style=\"width:100\%\">\n"                       \
	"   <tr bgcolor=\"LIGHTGREY\">\n"                         \
	"    <th style=\"padding:3px\" align=\"right\"><pre>Id</pre></th>\n"            \
	"    <th style=\"padding:3px\" align=\"left\"><pre>Type</pre></th>\n"           \
	"    <th style=\"padding:3px\" align=\"left\"><pre>Timestamp</pre></th>\n"      \
	"    <th style=\"padding:3px\" align=\"left\"><pre>Timestamp IOC</pre></th>\n"  \
	"    <th style=\"padding:3px\" align=\"right\"><pre>TrigId</pre></th>\n"        \
	"    <th style=\"padding:3px\" align=\"left\"><pre>Text</pre></th>\n"           \
	"   </tr>\n";
static char log_html_doc_template_end[] =            \
	"  </table>\n"                                   \
	" </body>\n"                                     \
	"</html>";
static char log_html_row_template[] =                      \
	"  <tr bgcolor=\"%s\">"                                \
	"   <td style=\"padding:3px\" align=\"right\"><pre>%d</pre></td>"            \
	"   <td style=\"padding:3px\" align=\"left\"><pre>%s</pre></td>"             \
	"   <td style=\"padding:3px\" align=\"left\"><pre>%s</pre></td>"             \
	"   <td style=\"padding:3px\" align=\"left\"><pre>%s</pre></td>"             \
	"   <td style=\"padding:3px\" align=\"right\"><pre>%d</pre></td>"            \
	"   <td style=\"padding:3px\" align=\"left\"><pre>%s%s%s%s%s%s%s</pre></td>" \
	"  </tr>\n";

// maps that hold data we parse out of the XML doc
static map<int, pair<string, string> > matrix_items;
static map<int, map<int, string> > log_text;

// this list is not defined in Resource.xml
static const char *event_info_type_strs[] = {
	"State",
	"Warning",
	"Interlock",
	"Error",
	"Param",
	"Message"
};

static const char *event_info_type_colors[] = {
	"DARKGREY",
	"YELLOW",
	"RED",
	"RED",
	"DARKGREY",
	"DARKGREY"
};

// this list is not defined in Resource.xml
static const char *event_info_data_type_strs[] = {
	"No data",
	"Real",
	"Bool",
	"Int",
	"Uint",
	"Word",
	"Dint",
	"Udint",
	"Dword"
};


// enable or disable debug printfs
static void
set_event_debug(const int on)
{
	debug_flag = on;
	printf("debug %s\n", debug_flag ? "on" : "off");
}

// given a node in XML, find a subnode by name
static xmlNode *
xml_get_node_by_name(xmlNode *node, char *name)
{
	xmlNode *subnode = node->children;

	while (subnode != NULL) {
		if (xmlStrcmp(subnode->name, (xmlChar *) name) == 0) {
			return subnode;
		}
		subnode = subnode->next;
	}

	return NULL;
}

// given a node in XML, find the Name and Unit subnodes and retrieve their values
static void
xml_matrix_items(xmlNode *node)
{
	int index;
	xmlNode *index_node = NULL;
	xmlNode *name_node = NULL;
	xmlNode *unit_node = NULL;
	char *name = NULL;
	char *unit = NULL;

	for (index_node = node->children; index_node != NULL; index_node = index_node->next) {
		sscanf((char *) index_node->name, "Index%d", &index);

		name_node = xml_get_node_by_name(index_node, (char *) "Name");
		unit_node = xml_get_node_by_name(index_node, (char *) "Unit");

		name = (char *) xmlNodeGetContent(name_node);
		unit = (char *) xmlNodeGetContent(unit_node);

		matrix_items.insert(pair<int, pair<string, string> >(index, pair<string, string>(name, unit)));
	}
}

// given a node in XML, iterate the subnodes, find the subnodes whose names
// match "no%d" and retrieve their values.  the %d part of "no%d" is the
// index value that the modulator gives us during run time, mapping an int
// to a string.
static void
xml_by_number(xmlNode *node, map<int, string> &vals)
{
	xmlNode *number_node = NULL;
	xmlNode *text_node = NULL;
	int number = 0;

	for (number_node = node->children; number_node != NULL; number_node = number_node->next) {
		sscanf((char *) number_node->name, "no%d", &number);

		text_node = xml_get_node_by_name(number_node, (char *) "Text");
		if (!text_node) {
			printf("error: XML Text not found\n");
		}
		vals.insert(pair<int, string>(number, (char *) xmlNodeGetContent(text_node)));
	}
}

// we use libxml2 to parse XML docs
static void
parse_resources_xml(const char *name)
{
	xmlDocPtr doc = xmlReadFile(name, NULL, XML_PARSE_NOBLANKS);
	xmlNode *gui_node = NULL;
	xmlNode *matrix_items_node = NULL;
	xmlNode *strings_node = NULL;
	xmlNode *node = NULL;
	int i = 0;

	if (doc == NULL) {
		printf("error: XML %s not found\n", name);
		return;
	}

	gui_node = xmlDocGetRootElement(doc);
	if (!gui_node) {
		printf("error: XML %s missing data\n", name);
		return;
	}

	matrix_items_node = xml_get_node_by_name(gui_node, (char *) "MatrixItems");
	if (!matrix_items_node) {
		printf("error: XML MatrixItems not found\n");
		return;
	}
	xml_matrix_items(matrix_items_node);

	strings_node = xml_get_node_by_name(gui_node, (char *) "Strings");
	if (!strings_node) {
		printf("error: XML Strings not found\n");
		return;
	}

	for (i = 0; i < (int) (sizeof(event_info_type_strs) / sizeof(char *)); i++) {
		node = xml_get_node_by_name(strings_node, (char *) event_info_type_strs[i]);
		if (!node) {
			printf("error: XML %s not found\n", event_info_type_strs[i]);
		}
		xml_by_number(node, log_text[i]);
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();
}

// print out the subsystem names and the byte that represents them in the
// XML status arrays given to use by Scandinova.  we use this to create
// a JSON file used by the PyDM GUI.
static void
matrix_json(void)
{
	int i = 0;

	printf("[\n");
	for (i = 0; i < (int) matrix_items.size(); i++) {
		if (matrix_items[i].first.length()) {
			printf(
				"{\"SUBSYSTEM_NUM\":\"%d\",\"SUBSYSTEM\":\"%s\"}%s\n",
				i,
				matrix_items[i].first.c_str(),
				i != (int) matrix_items.size() - 1 ? "," : "");
		}
	}
	printf("]\n");
}


// take a UNIX epoch and milliseconds offset, and construct a human-readable
// string representing the date, time, and timezone.
//
// TODO: should we have the format string be an externally settable parameter?
static void
human_timestr(time_t epoch, int milliseconds, char *str, size_t max_out_len_bytes)
{
	struct tm *ts = NULL;
	char timestamp[64];
	char timezone[8];

	ts = localtime(&epoch);
	strftime(timestamp, sizeof(timestamp), "%a %Y-%m-%d %H:%M:%S", ts);
	strftime(timezone, sizeof(timezone), "%Z", ts);
	memset(str, 0, max_out_len_bytes);
	snprintf(
		str,
		max_out_len_bytes - 1,
		"%s.%03d %s",
		timestamp,
		milliseconds,
		timezone);
}

// construct a human-readable string from the current time (now).
static void
now_to_timestr(char *str, size_t max_out_len_bytes)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	human_timestr(tv.tv_sec, tv.tv_usec / 1000, str, max_out_len_bytes);
}

// take a raw epoch timestamp given to us by the Scandinova hardware, and
// convert it to a human-readable timestamp string.
static void
epoch_raw_to_timestr(uint64_t epoch_raw, char *str, size_t max_out_len_bytes)
{
	human_timestr(
		(time_t) EPOCH_RAW_TO_EPOCH(epoch_raw),
		(int) EPOCH_RAW_TO_MILLISECONDS_OFFSET(epoch_raw),
		str,
		max_out_len_bytes);
}

static void
set_event_output_file(const char *name)
{
	if (event_output_file) {
		free(event_output_file);
	}
	if (event_output_fd) {
		fclose(event_output_fd);
	}

	event_output_file = strdup(name);
	event_output_fd = fopen(event_output_file, "r+");
	if (event_output_fd == NULL) {
		event_output_fd = fopen(event_output_file, "w+");
		if (event_output_fd == NULL) {
			printf("error: unable to write \'%s\': %s\n", event_output_file, strerror(errno));
			return;
		}
		fprintf(event_output_fd, log_html_doc_template_start);
		fprintf(event_output_fd, log_html_doc_template_end);
		fflush(event_output_fd);
	}
}

static void
write_event_output_file(
	int increment,
	int type,
	char *type_str,
	char *timestamp,
	char *timestamp_ioc,
	int trig_id,
	char *subsystem,
	char *text,
	char *data,
	char *units)
{
	int loc = strlen(log_html_doc_template_end);

	if (!event_output_fd) {
		printf("error: can't write \'%s\'\n", event_output_file);
		return;
	}

	fseek(event_output_fd, (long) -loc, SEEK_END);
	fprintf(
		event_output_fd,
		log_html_row_template,
		event_info_type_colors[type],
		increment,
		type_str,
		timestamp,
		timestamp_ioc,
		trig_id,
		subsystem,
		subsystem[0] ? " " : "",
		text,
		data[0] ? ": " : "",
		data,
		units[0] ? " " : "",
		units);
	fprintf(event_output_fd, log_html_doc_template_end);
	fflush(event_output_fd);
}

// convert raw bytes from the modulator to a meaningful number
static void
event_data_to_str(uint32_t data, int type, char *out, size_t max_out_len_bytes)
{
	memset(out, 0, max_out_len_bytes);

	switch (type) {
	case 0: // no data
		break;
	case 1: // real
		assert(sizeof(float) == sizeof(uint32_t));
		snprintf(out, max_out_len_bytes - 1, "%f", *(float *) (void *) &data);
		break;
	case 2: // bool
	case 3: // int
	case 5: // word
	case 6: // dint
		snprintf(out, max_out_len_bytes - 1, "%d", (int32_t) data);
		break;
	case 4: // uint
	case 7: // udint
	case 8: // dword
		snprintf(out, max_out_len_bytes - 1, "%u", (uint32_t) data);
		break;
	}
}

// if the event log is updating in hardware, we must wait for it to reach a
// consistent state before we pull data out of it.  so we delay by one second,
// and during that time if there are no further updates, we lock our
// representation of the event data and assume it is consistent.  then we
// write to the log file and unlock the event data.
//
// this function tells us if the representation of the log data is ok to use.
// return 0 ==> ok to write to log file
// return 1 ==> not ok to write to log file
//
// this function assumes the in-memory representation of the log data has
// been locked, i.e. the in-memory log will not change again until it has
// been unlocked.
static int is_log_updating(void)
{
	struct timespec now;
	struct timespec diff;

	clock_gettime(CLOCK_REALTIME, &now);

	diff.tv_sec = next_clean_time.tv_sec - now.tv_sec;
	diff.tv_nsec = next_clean_time.tv_nsec - now.tv_nsec;

	// if modulator event log has not changed for 1.0+ seconds
	if (diff.tv_sec < 0) {
		return 0;
	}
	if (diff.tv_sec == 0 && diff.tv_nsec < 0) {
		return 0;
	}

	// modulator event log updated within the previous 1 second
	return 1;
}

// check every second to see if the modulator's event log has new
// data that needs to be written to the log file.
static void *
update_log_file(void *unused)
{
	int previous_event_index = -1;
	int event_count = 0;
	event_info_t *event_info = NULL;
	int i = 0;

	while (1) {
		sleep(1);

		pthread_mutex_lock(&log_mutex);

		if (is_log_updating()) {
			// wait a little bit longer
			pthread_mutex_unlock(&log_mutex);
			continue;
		}

		if (previous_event_index == -1) {
			// initializing
			event_count = 0;
		} else if (previous_event_index < current_event_index) {
			// event log did not wrap
			event_count = current_event_index - previous_event_index;
		} else if (previous_event_index > current_event_index) {
			// event log wrapped
			event_count = current_event_index + (EVENT_LOG_SIZE - previous_event_index);
		} else {
			event_count = 0;
		}

		// write all the new events to a file
		for (i = 0; i < event_count; i++) {
			// previous_event_index contains the index of the event we previously
			// wrote to file.  so the next (unwritten) event is at +1, and we
			// have event_count of them.
			event_info = &event_infos[(previous_event_index + i + 1) % EVENT_LOG_SIZE];

			if (debug_flag) {
				printf("log file  ------------------------------------\n");
				printf("increment:     %d\n",                     event_info->increment);
				printf("trigger id:    %d\n",                     event_info->trigger);
				printf("timestamp:     %s\n",                     event_info->timestamp);
				printf("IOC timestamp: %s\n",                     event_info->timestamp_ioc);
				printf("type:          \"%s\" (%d)\n",            event_info->type_str, event_info->type);
				printf("subsystem:     \"%s\" (%d)\n",            event_info->subsystem_str, event_info->matrix_index);
				printf("log text:      \"%s\" (%d/%d)\n",         event_info->text_str, event_info->type, event_info->text_number);
				printf("data type:     \"%s\" (%d)\n",            event_info->data_type_str, event_info->data_type);
				printf("data:          \"%s\" \"%s\" (0x%08x)\n", event_info->data_str, event_info->units_str, event_info->data);
				printf("\n");
			}

			write_event_output_file(
				event_info->increment,
				event_info->type,
				event_info->type_str,
				event_info->timestamp,
				event_info->timestamp_ioc,
				event_info->trigger,
				event_info->subsystem_str,
				event_info->text_str,
				event_info->data_str,
				event_info->units_str);
		}

		previous_event_index = current_event_index;

		pthread_mutex_unlock(&log_mutex);
	}

	return NULL;
}

// convert raw event struct from hardware to a meaningful event
static void
event_struct_to_event_info(event_struct_t *event_struct, event_info_t *event_info)
{
	assert(event_struct->type < sizeof(event_info_type_strs) / sizeof(char *));
	assert(event_struct->data_type < sizeof(event_info_data_type_strs) / sizeof(char *));

	memset(event_info, 0, sizeof(event_info_t));

	event_info->epoch = EPOCH_RAW_TO_EPOCH(event_struct->epoch_raw);
	event_info->increment = event_struct->increment;
	event_info->type = event_struct->type;
	event_info->trigger = event_struct->trigger;
	event_info->text_number = event_struct->text_number;
	event_info->data_type = event_struct->data_type;
	event_info->data = event_struct->data;
	event_info->matrix_index = event_struct->matrix_index;

	strncpy(event_info->type_str, event_info_type_strs[event_info->type], sizeof(event_info->type_str) - 1);
	strncpy(event_info->text_str, log_text[event_info->type][event_info->text_number].c_str(), sizeof(event_info->text_str) - 1);
	strncpy(event_info->subsystem_str, matrix_items[event_info->matrix_index].first.c_str(), sizeof(event_info->subsystem_str));
	strncpy(event_info->units_str, matrix_items[event_info->matrix_index].second.c_str(), sizeof(event_info->units_str));
	strncpy(event_info->data_type_str, event_info_data_type_strs[event_info->data_type], sizeof(event_info->data_type_str) - 1);

	event_data_to_str(event_info->data, event_info->data_type, event_info->data_str, sizeof(event_info->data_str));

	epoch_raw_to_timestr(event_struct->epoch_raw, event_info->timestamp, sizeof(event_info->timestamp));
	now_to_timestr(event_info->timestamp_ioc, sizeof(event_info->timestamp_ioc));

	snprintf(
		event_info->display_str,
		sizeof(event_info->display_str) - 1,
		"%s%s%s%s%s%s%s",
		event_info->subsystem_str,
		event_info->subsystem_str[0] ? " " : "",
		event_info->text_str,
		event_info->data_str[0] ? ": " : "",
		event_info->data_str,
		event_info->units_str[0] ? " " : "",
		event_info->units_str);

}

// one of the arrays containing log data has changed.  here we handle the
// changing data.  in this function we build our in-memory representation
// of the log.  in another thread, we write each log item to a file.
//
// inputs from modbus (offsets and names from ScandiNova spec)
// INPA event index       EventIndex      read999       "Event logg index"
// INPB event increment   EventIncrement  read1000/50   "Event logg increment array"
// INPC timestamp         EventTime1      read1100/100  "Event logg time-stamp array"
// INPD timestamp         EventTime2      read1200/100  "Event logg time-stamp array"
// INPE event type        EventType       read1050/50   "Event logg type array"
// INPF event trigger     EventTrigger    read1300/100  "Event logg trig id array"
// INPG matrix index      EventCause      read1400/50   "Event logg index array"
// INPH event text number EventTextNum    read1450/50   "Event logg text number array"
// INPI data type         EventDataType   read1500/50   "Event logg data type array"
// INPJ data              EventData       read1550/100  "Event logg data array"
static long
handle_event_log_modify(aSubRecord *prec)
{
	int event_index = 0;
	uint32_t upper_32 = 0;
	uint32_t lower_32 = 0;
	event_info_t *event_info = NULL;
	event_struct_t event_struct;

	memset(&event_struct, 0, sizeof(event_struct));

	pthread_mutex_lock(&log_mutex);

	current_event_index = *(int *) prec->a;
	assert(current_event_index >= 0);
	assert(current_event_index < EVENT_LOG_SIZE);

	for (event_index = 0; event_index < EVENT_LOG_SIZE; event_index++) {
		event_info = &event_infos[event_index];

		// event timestamp

		if (event_index < EVENT_LOG_SIZE / 2) {
			upper_32 = ((uint32_t *) prec->c)[event_index * 2 + 1];
			lower_32 = ((uint32_t *) prec->c)[event_index * 2];
		} else {
			upper_32 = ((uint32_t *) prec->d)[(event_index - EVENT_LOG_SIZE / 2) * 2 + 1];
			lower_32 = ((uint32_t *) prec->d)[(event_index - EVENT_LOG_SIZE / 2) * 2];
		}

		event_struct.increment = (uint16_t) ((uint32_t *) prec->b)[event_index];
		event_struct.epoch_raw = ((uint64_t) upper_32 << 32) | lower_32;
		event_struct.type = (uint16_t) ((uint32_t *) prec->e)[event_index];
		event_struct.trigger = ((uint32_t *) prec->f)[event_index];
		event_struct.matrix_index = (uint16_t) ((uint32_t *) prec->g)[event_index];
		event_struct.text_number = (uint16_t) ((uint32_t *) prec->h)[event_index];
		event_struct.data_type = (uint16_t) ((uint32_t *) prec->i)[event_index];
		event_struct.data = ((uint32_t *) prec->j)[event_index];

		event_struct_to_event_info(&event_struct, event_info);

#if 0
		printf("increment:     %d\n",                     event_info->increment);
		printf("trigger id:    %d\n",                     event_info->trigger);
		printf("timestamp:     %s\n",                     event_info->timestamp);
		printf("IOC timestamp: %s\n",                     event_info->timestamp_ioc);
		printf("type:          \"%s\" (%d)\n",            event_info->type_str, event_info->type);
		printf("subsystem:     \"%s\" (%d)\n",            event_info->subsystem_str, event_info->matrix_index);
		printf("log text:      \"%s\" (%d/%d)\n",         event_info->text_str, event_info->type, event_info->text_number);
		printf("data type:     \"%s\" (%d)\n",            event_info->data_type_str, event_info->data_type);
		printf("data:          \"%s\" \"%s\" (0x%08x)\n", event_info->data_str, event_info->units_str, event_info->data);
		printf("\n");
#endif
	}

	// the time at which the event data is done being updated (clean)
	clock_gettime(CLOCK_REALTIME, &next_clean_time);
	next_clean_time.tv_sec++;

	pthread_mutex_unlock(&log_mutex);

	return 0;
}
epicsRegisterFunction(handle_event_log_modify);

// convert raw data from modbus to a meaningful event
static void
modbus_to_event_info(uint32_t *modbus_data, event_info_t *event_info)
{
	unsigned int i = 0;
	uint16_t words[sizeof(event_struct_t) / 2];

	for (i = 0; i < sizeof(event_struct_t) / 2; i++) {
		words[i] = (uint16_t) modbus_data[i];
	}

	event_struct_to_event_info((event_struct_t *) words, event_info);
}

// update current event struct PVs.
static void
handle_current_event(aSubRecord *prec, event_info_t *event_info)
{
	memset(prec->vala, 0, prec->nova);
	memset(prec->valb, 0, prec->novb);
	memset(prec->valc, 0, prec->novc);
	memset(prec->vald, 0, sizeof(long) * prec->novd);
	memset(prec->vale, 0, prec->nove);
	memset(prec->valf, 0, prec->novf);
	memset(prec->valg, 0, prec->novg);
	memset(prec->valh, 0, prec->novh);
	memset(prec->vali, 0, prec->novi);
	memset(prec->valj, 0, prec->novj);

	strncpy((char *) prec->vala, event_info->timestamp, prec->nova - 1);
	strncpy((char *) prec->valb, event_info->timestamp, prec->novb - 1);
	strncpy((char *) prec->valc, event_info->type_str, prec->novc - 1);
	*(long *) prec->vald = event_info->trigger;
	strncpy((char *) prec->vale, event_info->subsystem_str, prec->nove - 1);
	strncpy((char *) prec->valf, event_info->text_str, prec->novf - 1);
	strncpy((char *) prec->valg, event_info->units_str, prec->nog - 1);
	strncpy((char *) prec->valh, event_info->data_str, prec->noh - 1);
	strncpy((char *) prec->vali, event_info->display_str, prec->novi - 1);
}

// update warning event struct PVs.
//
// we discovered that the modulator's internal logic overwrites warning events
// with the next event (whatever kind of event) within the modbus arrays that
// hold 50 events.  so what we do here is preserve the warning from the event
// struct.  without this, we would have no trace of a warning event ever
// happening.
static void
handle_warning_event(aSubRecord *prec, event_info_t *event_info)
{
	memset(prec->valj, 0, prec->novj);
	memset(prec->valk, 0, prec->novk);
	memset(prec->vall, 0, prec->novl);
	memset(prec->valm, 0, sizeof(long) * prec->novm);
	memset(prec->valn, 0, prec->novn);
	memset(prec->valo, 0, prec->novo);
	memset(prec->valp, 0, prec->novp);
	memset(prec->valq, 0, prec->novq);
	memset(prec->valr, 0, prec->novr);

	strncpy((char *) prec->valj, event_info->timestamp, prec->novj - 1);
	strncpy((char *) prec->valk, event_info->timestamp, prec->novk - 1);
	strncpy((char *) prec->vall, event_info->type_str, prec->novl - 1);
	*(long *) prec->valm = event_info->trigger;
	strncpy((char *) prec->valn, event_info->subsystem_str, prec->novn - 1);
	strncpy((char *) prec->valo, event_info->text_str, prec->novo - 1);
	strncpy((char *) prec->valp, event_info->units_str, prec->novp - 1);
	strncpy((char *) prec->valq, event_info->data_str, prec->novq - 1);
	strncpy((char *) prec->valr, event_info->display_str, prec->novr - 1);
}


// inputs from modbus:
// INPA CurrentEventStruct read1700
//
// outputs to EPICS:
// OUTA CurrentEventTimeStr
// OUTB CurrentEventIocTimeStr
// OUTC CurrentEventTypeStr
// OUTD CurrentEventTrigger
// OUTE CurrentEventSubsystemStr
// OUTF CurrentEventTextStr
// OUTG CurrentEventUnitStr
// OUTH CurrentEventDataStr
// OUTI CurrentEventDisplayStr
// OUTJ WarningEventTimeStr
// OUTK WarningEventIocTimeStr
// OUTL WarningEventTypeStr
// OUTM WarningEventTrigger
// OUTN WarningEventSubsystemStr
// OUTO WarningEventTextStr
// OUTP WarningEventUnitStr
// OUTQ WarningEventDataStr
// OUTR WarningEventDisplayStr
static long
handle_current_event_struct_modify(aSubRecord *prec)
{
	event_info_t event_info;

	memset(&event_info, 0, sizeof(event_info));
	modbus_to_event_info((uint32_t *) prec->a, &event_info);

	if (event_info.type != EVENT_TYPE_WARNING) {
		handle_current_event(prec, &event_info);
	} else {
		handle_warning_event(prec, &event_info);
	}

	if (debug_flag) {
		printf("%s event ------------------------------------\n", event_info_type_strs[event_info.type]);
		printf("increment:     %d\n",                     event_info.increment);
		printf("trigger id:    %d\n",                     event_info.trigger);
		printf("timestamp:     %s\n",                     event_info.timestamp);
		printf("IOC timestamp: %s\n",                     event_info.timestamp_ioc);
		printf("type:          \"%s\" (%d)\n",            event_info.type_str, event_info.type);
		printf("subsystem:     \"%s\" (%d)\n",            event_info.subsystem_str, event_info.matrix_index);
		printf("log text:      \"%s\" (%d/%d)\n",         event_info.text_str, event_info.type, event_info.text_number);
		printf("data type:     \"%s\" (%d)\n",            event_info.data_type_str, event_info.data_type);
		printf("data:          \"%s\" \"%s\" (0x%08x)\n", event_info.data_str, event_info.units_str, event_info.data);
		printf("\n");
	}

	return 0;
}
epicsRegisterFunction(handle_current_event_struct_modify);

// inputs from modbus:
// INPA InterlockEventStruct read1715
//
// outputs to EPICS:
// OUTA InterlockEventTimeStr
// OUTB InterlockEventIocTimeStr
// OUTC InterlockEventTypeStr
// OUTD InterlockEventTrigger
// OUTE InterlockEventSubsystemStr
// OUTF InterlockEventTextStr
// OUTG InterlockEventUnitStr
// OUTH InterlockEventDataStr
// OUTI InterlockEventDisplayStr
static long
handle_interlock_event_struct_modify(aSubRecord *prec)
{
	event_info_t event_info;

	modbus_to_event_info((uint32_t *) prec->a, &event_info);

	memset(prec->vala, 0, prec->nova);
	memset(prec->valb, 0, prec->novb);
	memset(prec->valc, 0, prec->novc);
	memset(prec->vald, 0, sizeof(long) * prec->novd);
	memset(prec->vale, 0, prec->nove);
	memset(prec->valf, 0, prec->novf);
	memset(prec->valg, 0, prec->novg);
	memset(prec->valh, 0, prec->novh);
	memset(prec->vali, 0, prec->novi);

	strncpy((char *) prec->vala, event_info.timestamp, prec->nova - 1);
	strncpy((char *) prec->valb, event_info.timestamp_ioc, prec->novb - 1);
	strncpy((char *) prec->valc, event_info.type_str, prec->novc - 1);
	*(long *) prec->vald = event_info.trigger;
	strncpy((char *) prec->vale, event_info.subsystem_str, prec->nove - 1);
	strncpy((char *) prec->valf, event_info.text_str, prec->novf - 1);
	strncpy((char *) prec->valg, event_info.units_str, prec->novg - 1);
	strncpy((char *) prec->valh, event_info.data_str, prec->novh - 1);
	strncpy((char *) prec->vali, event_info.display_str, prec->novi - 1);

	if (debug_flag) {
		printf("%s event ------------------------------\n", event_info_type_strs[event_info.type]);
		printf("increment:     %d\n",                     event_info.increment);
		printf("trigger id:    %d\n",                     event_info.trigger);
		printf("timestamp:     %s\n",                     event_info.timestamp);
		printf("IOC timestamp: %s\n",                     event_info.timestamp_ioc);
		printf("type:          \"%s\" (%d)\n",            event_info.type_str, event_info.type);
		printf("subsystem:     \"%s\" (%d)\n",            event_info.subsystem_str, event_info.matrix_index);
		printf("log text:      \"%s\" (%d/%d)\n",         event_info.text_str, event_info.type, event_info.text_number);
		printf("data type:     \"%s\" (%d)\n",            event_info.data_type_str, event_info.data_type);
		printf("data:          \"%s\" \"%s\" (0x%08x)\n", event_info.data_str, event_info.units_str, event_info.data);
		printf("\n");
	}

	return 0;
}
epicsRegisterFunction(handle_interlock_event_struct_modify);

// we enter this function when the user sets one of the values that tells us
// that the user wants to view a waveform.  there are four types of waveforms,
// and each type has a history of five waveforms.  anytime a type is enabled,
// we enable the download (via modbus) of all five historicals.  probably only
// one historical is presented to the user at a time.  I expect all four
// types would be displayed at the same time, however.
//
// the function that handles the waveform being ready to download causes the
// "next" waveform to begin downloading (from the PLC to the modbus buffer
// on Scandinova hardware).  but because at first there isn't a waveform
// ready to download, we must kick off the first download here in this
// function.
//
// inputs from EPICS:
// INPA read3000 "Waveform type"
// INPB read3001 "Waveform time"
// INPC read3009 "Sample count"
// INPD read3005 "Pulse ID"
//
// outputs to modbus:
// OUTA write3000 "WaveformTypeSet"
static long
handle_waveform_request(aSubRecord *prec)
{
	int cvd_enable = *(int *) prec->a;
	int ct_enable = *(int *) prec->b;
	int rf_fwd_enable = *(int *) prec->c;
	int rf_rfl_enable = *(int *) prec->d;
	uint64_t mask = 0;
	waveform_code_t kick_waveform = WAVEFORM_UNKNOWN;

	if (cvd_enable) {
		kick_waveform = WAVEFORM_CVD_T0_REQUEST;
		mask |=
			(1L << WAVEFORM_CVD_T0_REQUEST) | (1L << WAVEFORM_CVD_T1_REQUEST) |
			(1L << WAVEFORM_CVD_T2_REQUEST) | (1L << WAVEFORM_CVD_T3_REQUEST) |
			(1L << WAVEFORM_CVD_T4_REQUEST);
	} else {
		*(long *) prec->valb = 1;
	}

	if (ct_enable) {
		kick_waveform = WAVEFORM_CT_T0_REQUEST;
		mask |=
			(1L << WAVEFORM_CT_T0_REQUEST) | (1L << WAVEFORM_CT_T1_REQUEST) |
			(1L << WAVEFORM_CT_T2_REQUEST) | (1L << WAVEFORM_CT_T3_REQUEST) |
			(1L << WAVEFORM_CT_T4_REQUEST);
	} else {
		*(long *) prec->valc = 1;
	}

	if (rf_fwd_enable) {
		kick_waveform = WAVEFORM_RF_FWD_T0_REQUEST;
		mask |=
			(1L << WAVEFORM_RF_FWD_T0_REQUEST) | (1L << WAVEFORM_RF_FWD_T1_REQUEST) |
			(1L << WAVEFORM_RF_FWD_T2_REQUEST) | (1L << WAVEFORM_RF_FWD_T3_REQUEST) |
			(1L << WAVEFORM_RF_FWD_T4_REQUEST);
	} else {
		*(long *) prec->vald = 1;
	}

	if (rf_rfl_enable) {
		kick_waveform = WAVEFORM_RF_RFL_T0_REQUEST;
		mask |=
			(1L << WAVEFORM_RF_RFL_T0_REQUEST) | (1L << WAVEFORM_RF_RFL_T1_REQUEST) |
			(1L << WAVEFORM_RF_RFL_T2_REQUEST) | (1L << WAVEFORM_RF_RFL_T3_REQUEST) |
			(1L << WAVEFORM_RF_RFL_T4_REQUEST);
	} else {
		*(long *) prec->vale = 1;
	}

	if (debug_flag) {
		printf("waveform mask %08llx\n", (unsigned long long) mask);
	}

	// if no waveforms enabled and we have one that needs to be started
	if (!waveform_request_enable && kick_waveform != WAVEFORM_UNKNOWN) {
		*(long *) prec->vala = kick_waveform;
	}

	waveform_request_enable = mask;

	return 0;
}
epicsRegisterFunction(handle_waveform_request);

// a waveform is available in the modbus buffer on the Scandinova hardware.  in
// this function we read it from modbus into our buffers here in this app.
// then we tell the Scandinova hardware to download the next waveform, and when
// that download is finished we will again be in this function.
//
// inputs from modbus:
// INPA read3000 "Waveform type"
// INPB read3001 "Waveform time"
// INPC read3009 "Sample count"
// INPD read3005 "Pulse ID"
// INPE read3010/100 "Waveform"
// INPF read3110/100 "Waveform"
// INPG read3210/100 "Waveform"
// INPH read3310/100 "Waveform"
// INPI read3410/100 "Waveform"
// INPJ read3510/3 "Waveform"
//
// outputs to EPICS:
// OUTA WaveformTimeStrCvd
// OUTB WaveformSampleCountCvd
// OUTC WaveformPulseIdCvd
// OUTD WaveformCvd
// OUTE WaveformTimeStrCt
// OUTF WaveformSampleCountCt
// OUTG WaveformPulseIdCt
// OUTH WaveformCt
// OUTI WaveformTimeStrRfFwd
// OUTJ WaveformSampleCountRfFwd
// OUTK WaveformPulseIdRfFwd
// OUTL WaveformRfFwd
// OUTM WaveformTimeStrRfRfl
// OUTN WaveformSampleCountRfRfl
// OUTO WaveformPulseIdRfRfl
// OUTP WaveformRfRfl
// OUTQ write3000 "WaveformTypeSet"
static long
handle_waveform_ready(aSubRecord *prec)
{
	waveform_code_t waveform_ready_code = (waveform_code_t) *(long *) prec->a;
	waveform_code_t waveform_request_code = WAVEFORM_UNKNOWN;
	waveform_code_t i = WAVEFORM_UNKNOWN;
	char *timestamp = NULL;
	int *sample_count = NULL;
	int *trig_id = NULL;
	double *waveform = NULL;

	// if the state of the waveforms hasn't changed since the last time we were here
	if (prev_waveform_ready_code == waveform_ready_code) {
		return 0;
	}
	prev_waveform_ready_code = waveform_ready_code;

	// if the waveform has not completely downloaded yet
	if (waveform_ready_code == WAVEFORM_FETCHING) {
		return 0;
	}

	// XXX should we always have 503 samples?
	if (*(long *) prec->c != WAVEFORM_SAMPLE_COUNT_MAX) {
		return 0;
	}

	// sanity check our buffer sizes
	assert(prec->noe + prec->nof + prec->nog + prec->noh + prec->noi + prec->noj == prec->novd);
	assert(prec->novd == prec->novh);
	assert(prec->novd == prec->novl);
	assert(prec->novd == prec->novp);

	waveform_request_code = WAVEFORM_READY_CODE_TO_REQUEST_CODE(waveform_ready_code);
	assert(waveform_request_code > WAVEFORM_UNKNOWN);
	assert(waveform_request_code < WAVEFORM_COUNT_MAX);

	// convenience variables
	timestamp = waveform_timestamp[waveform_request_code];
	sample_count = &waveform_sample_counts[waveform_request_code];
	trig_id = &waveform_trig_ids[waveform_request_code];
	waveform = waveforms[waveform_request_code];

	// populate the local (to this app) waveform data buffers
	epoch_raw_to_timestr(
			*(uint64_t *) prec->b,
			timestamp,
			sizeof(waveform_timestamp[waveform_request_code]));
	*sample_count = *(long *) prec->c;
	*trig_id = *(long *) prec->d;
	memcpy(waveform +   0, (double *) prec->e, prec->noe);
	memcpy(waveform + 100, (double *) prec->f, prec->nof);
	memcpy(waveform + 200, (double *) prec->g, prec->nog);
	memcpy(waveform + 300, (double *) prec->h, prec->noh);
	memcpy(waveform + 400, (double *) prec->i, prec->noi);
	memcpy(waveform + 500, (double *) prec->j, prec->noj);

	// TODO: apply scaling factors to the waveform.
	// the scaling factors are not available to us at this time.
	// Scandinova will make them available via modbus in a future patch.

	// copy waveform data to PVs
	if (waveform_request_code <= WAVEFORM_CVD_T4_REQUEST) {
		strncpy((char *) prec->vala, timestamp, prec->nova);
		*(long *) prec->valb = *sample_count;
		*(long *) prec->valr = *sample_count;
		*(long *) prec->valc = *trig_id;
		memcpy(prec->vald, waveform, prec->novd * sizeof(waveform[0]));
	} else if (waveform_request_code <= WAVEFORM_CT_T4_REQUEST) {
		strncpy((char *) prec->vale, timestamp, prec->nove);
		*(long *) prec->valf = *sample_count;
		*(long *) prec->vals = *sample_count;
		*(long *) prec->valg = *trig_id;
		memcpy(prec->valh, waveform, prec->novh * sizeof(waveform[0]));
	} else if (waveform_request_code <= WAVEFORM_RF_FWD_T4_REQUEST) {
		strncpy((char *) prec->vali, timestamp, prec->novi);
		*(long *) prec->valj = *sample_count;
		*(long *) prec->valt = *sample_count;
		*(long *) prec->valk = *trig_id;
		memcpy(prec->vall, waveform, prec->novl * sizeof(waveform[0]));
	} else if (waveform_request_code <= WAVEFORM_RF_RFL_T4_REQUEST) {
		strncpy((char *) prec->valm, timestamp, prec->novm);
		*(long *) prec->valn = *sample_count;
		*(long *) prec->valu = *sample_count;
		*(long *) prec->valo = *trig_id;
		memcpy(prec->valp, waveform, prec->novp * sizeof(waveform[0]));
	} else {
		assert(0);
	}

	// calculate next waveform to request, if any
	for (
			i = (waveform_code_t) ((waveform_request_code + 1) % WAVEFORM_COUNT_MAX);
			i != waveform_request_code;
			i = (waveform_code_t) ((i + 1) % WAVEFORM_COUNT_MAX)) {

		if ((1 << i) & waveform_request_enable) {
			// request waveform
			*(long *) prec->valq = i;
			break;
		}
	}

	return 0;
}
epicsRegisterFunction(handle_waveform_ready);

// gather all the subsystem statuses.  modbus gives us a status bit array
// and an subsystem state array.  status bits give us ok/warning/interlock
// for each subsystem, and state bits tell us which state the subsystems are
// in.
//
// we know the names of the subsystems by parsing them out from the XML file
// given to us by Scandinova.  the byte location matches the Index in the XML.
//
// inputs from modbus:
// INPA read2300/100 "status bit array"
// INPB read2400/100 "status bit array"
// INPC read2500/56  "status bit array"
// INPD read2000/100 "state read"
// INPE read2100/100 "state read"
// INPF read2200/56  "state read"
//
// outputs to EPICS:
// OUTA WarningStatus
// OUTB InterlockStatus
// OUTC SubsystemState
static long
handle_subsystem_status_modify(aSubRecord *prec)
{
	int i = 0;
	uint32_t *status1 = (uint32_t *) prec->a;
	uint32_t *status2 = (uint32_t *) prec->b;
	uint32_t *status3 = (uint32_t *) prec->c;
	uint32_t *state1 = (uint32_t *) prec->d;
	uint32_t *state2 = (uint32_t *) prec->e;
	uint32_t *state3 = (uint32_t *) prec->f;
	uint8_t *warning_status = (uint8_t *) prec->vala;
	uint8_t *interlock_status = (uint8_t *) prec->valb;
	uint8_t *subsystem_state = (uint8_t *) prec->valc;

	assert(prec->noa == 100);
	assert(prec->nob == 100);
	assert(prec->noc == 56);
	assert(prec->nod == 100);
	assert(prec->noe == 100);
	assert(prec->nof == 56);
	assert(prec->nova == 256);
	assert(prec->novb == 256);
	assert(prec->novc == 256);

	memset(warning_status, 0, prec->nova);
	memset(interlock_status, 0, prec->novb);
	memset(subsystem_state, 0, prec->novc);

	for (i = 0; i < (int) prec->noa; i++) {
		warning_status[i] = status1[i] & 1;
		warning_status[i + 100]   = status2[i] & 1;
		interlock_status[i]       = (status1[i] & 2) > 0;
		interlock_status[i + 100] = (status2[i] & 2) > 0;
		subsystem_state[i]        = state1[i];
		subsystem_state[i + 100]  = state2[i];
		if (i < 56) {
			warning_status[i + 200]   = status3[i] & 1;
			interlock_status[i + 200] = (status3[i] & 2) > 0;
			subsystem_state[i + 200]  = state3[i];
		}
	}

	return 0;
}
epicsRegisterFunction(handle_subsystem_status_modify);

extern "C"
{

static const iocshArg matrix_json_configure_arg0 = {"", iocshArgInt};
static const iocshArg *matrix_json_configure_args[] = {&matrix_json_configure_arg0};
static const iocshFuncDef matrix_json_func_def = {"matrix_json", 1, matrix_json_configure_args};
static void matrix_json_call_func(const iocshArgBuf *args)
{
	matrix_json();
}

static const iocshArg event_debug_configure_arg0 = {"0|1", iocshArgInt};
static const iocshArg *event_debug_configure_args[] = {&event_debug_configure_arg0};
static const iocshFuncDef event_debug_func_def = {"set_event_debug", 1, event_debug_configure_args};
static void event_debug_call_func(const iocshArgBuf *args)
{
	set_event_debug(args[0].ival);
}

static const iocshArg event_resources_configure_arg0 = {"filename", iocshArgString};
static const iocshArg *event_resources_configure_args[] = {&event_resources_configure_arg0};
static const iocshFuncDef event_resources_func_def = {"parse_resources_xml", 1, event_resources_configure_args};
static void event_resources_call_func(const iocshArgBuf *args)
{
	parse_resources_xml(args[0].sval);
}

static const iocshArg event_log_configure_arg0 = {"filename", iocshArgString};
static const iocshArg *event_log_configure_args[] = {&event_log_configure_arg0};
static const iocshFuncDef event_log_func_def = {"set_event_output_file", 1, event_log_configure_args};
static void event_log_call_func(const iocshArgBuf *args)
{
	set_event_output_file(args[0].sval);
}

void event_log_register_commands(void)
{
	pthread_create(&log_thread, NULL, update_log_file, NULL);
	pthread_detach(log_thread);

	iocshRegister(&matrix_json_func_def, matrix_json_call_func);
	iocshRegister(&event_debug_func_def, event_debug_call_func);
	iocshRegister(&event_log_func_def, event_log_call_func);
	iocshRegister(&event_resources_func_def, event_resources_call_func);
}
epicsExportRegistrar(event_log_register_commands);

}
