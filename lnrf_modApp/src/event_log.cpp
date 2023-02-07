#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
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


using namespace std;


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
	char timestamp[64];     // human-readable timestamp
	int trigger;            // counter
	int type;               // index for Strings and event types
	char type_str[32];      // type description
	int text_number;        // index for Strings[type]
	char text_str[64];      // from Strings[type][text_number]
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
	event_output_fd = fopen(event_output_file, "a");
	if (event_output_fd == NULL) {
		printf("error: unable to write \'%s\': %s\n", event_output_file, strerror(errno));
	}
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
				printf("increment:  %d\n",                     event_info->increment);
				printf("trigger id: %d\n",                     event_info->trigger);
				printf("timestamp:  %s\n",                     event_info->timestamp);
				printf("type:       \"%s\" (%d)\n",            event_info->type_str, event_info->type);
				printf("subsystem:  \"%s\" (%d)\n",            event_info->subsystem_str, event_info->matrix_index);
				printf("log text:   \"%s\" (%d/%d)\n",         event_info->text_str, event_info->type, event_info->text_number);
				printf("data type:  \"%s\" (%d)\n",            event_info->data_type_str, event_info->data_type);
				printf("data:       \"%s\" \"%s\" (0x%08x)\n", event_info->data_str, event_info->units_str, event_info->data);
				printf("\n");
			}

			if (event_output_fd != NULL) {
				// if this event has an empty data field
				if (event_info->data_type == 0) {
					fprintf(event_output_fd, "%d \"%s\" %3d %-9s \"%s\" \"%s\"\n",
						event_info->increment,
						event_info->timestamp,
						event_info->trigger,
						event_info->type_str,
						event_info->subsystem_str,
						event_info->text_str);
				} else {
					fprintf(event_output_fd, "%d \"%s\" %3d %-9s \"%s\" \"%s: %s %s\"\n",
						event_info->increment,
						event_info->timestamp,
						event_info->trigger,
						event_info->type_str,
						event_info->subsystem_str,
						event_info->text_str,
						event_info->data_str,
						event_info->units_str);
				}
				fflush(event_output_fd);
			}
		}

		previous_event_index = current_event_index;

		pthread_mutex_unlock(&log_mutex);
	}

	return NULL;
}

// take a raw epoch timestamp given to us by the Scandinova hardware, and
// convert it to a human-readable timestamp string.
static void
epoch_raw_to_timestr(uint64_t epoch_raw, char *str, size_t max_out_len_bytes)
{
	struct tm *ts = NULL;
	char timestamp[64];
	char timezone[8];
	time_t epoch = EPOCH_RAW_TO_EPOCH(epoch_raw);

	ts = localtime(&epoch);
	strftime(timestamp, sizeof(timestamp), "%a %Y-%m-%d %H:%M:%S", ts);
	strftime(timezone, sizeof(timezone), "%Z", ts);
	memset(str, 0, max_out_len_bytes);
	snprintf(
		str,
		max_out_len_bytes - 1,
		"%s.%03d %s",
		timestamp,
		(int) EPOCH_RAW_TO_MILLISECONDS_OFFSET(epoch_raw),
		timezone);
}

// convert raw event struct from hardware to a meaningful event
static void
event_struct_to_event_info(event_struct_t *event_struct, event_info_t *event_info)
{
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
		printf("increment:  %d\n",                     event_info->increment);
		printf("trigger id: %d\n",                     event_info->trigger);
		printf("timestamp:  %s\n",                     event_info->timestamp);
		printf("type:       \"%s\" (%d)\n",            event_info->type_str, event_info->type);
		printf("subsystem:  \"%s\" (%d)\n",            event_info->subsystem_str, event_info->matrix_index);
		printf("log text:   \"%s\" (%d/%d)\n",         event_info->text_str, event_info->type, event_info->text_number);
		printf("data type:  \"%s\" (%d)\n",            event_info->data_type_str, event_info->data_type);
		printf("data:       \"%s\" \"%s\" (0x%08x)\n", event_info->data_str, event_info->units_str, event_info->data);
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

// inputs from modbus:
// INPA CurrentEventStruct read1700
//
// outputs to EPICS:
// OUTA CurrentEventTimeStr
// OUTB CurrentEventTypeStr
// OUTC CurrentEventTrigger
// OUTD CurrentEventSubsystemStr
// OUTE CurrentEventTextStr
// OUTF CurrentEventTextStr2
// OUTG CurrentEventUnitStr
// OUTH CurrentEventDataStr
static long
handle_current_event_struct_modify(aSubRecord *prec)
{
	event_info_t event_info;

	modbus_to_event_info((uint32_t *) prec->a, &event_info);

	memset(prec->vala, 0, prec->nova);
	memset(prec->valb, 0, prec->novb);
	memset(prec->valc, 0, sizeof(long) * prec->novc);
	memset(prec->vald, 0, prec->novd);
	memset(prec->vale, 0, prec->nove);
	memset(prec->valf, 0, prec->novf);
	memset(prec->valg, 0, prec->novg);
	memset(prec->valh, 0, prec->novh);

	strncpy((char *) prec->vala, event_info.timestamp, prec->nova - 1);
	strncpy((char *) prec->valb, event_info.type_str, prec->novb - 1);
	*(long *) prec->valc = event_info.trigger;
	strncpy((char *) prec->vald, event_info.subsystem_str, prec->novd - 1);
	strncpy((char *) prec->vale, event_info.text_str, prec->nove - 1);
	strncpy((char *) prec->valf, event_info.subsystem_str, prec->novf - 1);
	strncpy((char *) prec->valg, event_info.units_str, prec->novg - 1);
	strncpy((char *) prec->valh, event_info.data_str, prec->novh - 1);

	if (debug_flag) {
		printf("event ------------------------------------\n");
		printf("increment:  %d\n",                     event_info.increment);
		printf("trigger id: %d\n",                     event_info.trigger);
		printf("timestamp:  %s\n",                     event_info.timestamp);
		printf("type:       \"%s\" (%d)\n",            event_info.type_str, event_info.type);
		printf("subsystem:  \"%s\" (%d)\n",            event_info.subsystem_str, event_info.matrix_index);
		printf("log text:   \"%s\" (%d/%d)\n",         event_info.text_str, event_info.type, event_info.text_number);
		printf("data type:  \"%s\" (%d)\n",            event_info.data_type_str, event_info.data_type);
		printf("data:       \"%s\" \"%s\" (0x%08x)\n", event_info.data_str, event_info.units_str, event_info.data);
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
// OUTB InterlockEventTypeStr
// OUTC InterlockEventTrigger
// OUTD InterlockEventSubsystemStr
// OUTE InterlockEventTextStr
// OUTF InterlockEventTextStr2
// OUTG InterlockEventUnitStr
// OUTH InterlockEventDataStr
static long
handle_interlock_event_struct_modify(aSubRecord *prec)
{
	event_info_t event_info;

	modbus_to_event_info((uint32_t *) prec->a, &event_info);

	memset(prec->vala, 0, prec->nova);
	memset(prec->valb, 0, prec->novb);
	memset(prec->valc, 0, sizeof(long) * prec->novc);
	memset(prec->vald, 0, prec->novd);
	memset(prec->vale, 0, prec->nove);
	memset(prec->valf, 0, prec->novf);
	memset(prec->valg, 0, prec->novg);
	memset(prec->valh, 0, prec->novh);

	strncpy((char *) prec->vala, event_info.timestamp, prec->nova - 1);
	strncpy((char *) prec->valb, event_info.type_str, prec->novb - 1);
	*(long *) prec->valc = event_info.trigger;
	strncpy((char *) prec->vald, event_info.subsystem_str, prec->novd - 1);
	strncpy((char *) prec->vale, event_info.text_str, prec->nove - 1);
	strncpy((char *) prec->valf, event_info.subsystem_str, prec->novf - 1);
	strncpy((char *) prec->valg, event_info.units_str, prec->novg - 1);
	strncpy((char *) prec->valh, event_info.data_str, prec->novh - 1);

	if (debug_flag) {
		printf("interlock ------------------------------------\n");
		printf("increment:  %d\n",                     event_info.increment);
		printf("trigger id: %d\n",                     event_info.trigger);
		printf("timestamp:  %s\n",                     event_info.timestamp);
		printf("type:       \"%s\" (%d)\n",            event_info.type_str, event_info.type);
		printf("subsystem:  \"%s\" (%d)\n",            event_info.subsystem_str, event_info.matrix_index);
		printf("log text:   \"%s\" (%d/%d)\n",         event_info.text_str, event_info.type, event_info.text_number);
		printf("data type:  \"%s\" (%d)\n",            event_info.data_type_str, event_info.data_type);
		printf("data:       \"%s\" \"%s\" (0x%08x)\n", event_info.data_str, event_info.units_str, event_info.data);
		printf("\n");
	}

	return 0;
}
epicsRegisterFunction(handle_interlock_event_struct_modify);

// inputs from modbus:
// INPA  read3001/4 "Customer waveform timestamp"
//
// outputs to EPICS:
// OUTA WaveformTimeStr
//
// note this is unrelated to logging and events.  we put it in this file for
// convenience only.
static long
handle_waveform_timestamp_modify(aSubRecord *prec)
{
	epoch_raw_to_timestr(*(uint64_t *) prec->a, (char *) prec->vala, prec->nova);

	return 0;
}
epicsRegisterFunction(handle_waveform_timestamp_modify);

extern "C"
{

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

	iocshRegister(&event_debug_func_def, event_debug_call_func);
	iocshRegister(&event_log_func_def, event_log_call_func);
	iocshRegister(&event_resources_func_def, event_resources_call_func);
}
epicsExportRegistrar(event_log_register_commands);

}
