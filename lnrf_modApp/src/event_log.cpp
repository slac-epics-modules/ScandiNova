#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <iocsh.h>
#include <epicsTypes.h>
#include <aSubRecord.h>
#include <registryFunction.h>
#include <epicsExport.h>
#include "asynPortDriver.h"


typedef struct {
	time_t epoch;
	char timestamp[64];
	int type;
	char type_str[32];
	int trigger;
	int cause_index;
	char cause_str[64];
	int text_number;
	char text_number_str[32];
	int data_type;
	char data_type_str[16];
	uint32_t data;
	char data_str[32];
} event_info_t;


static int event_index = -1;
static event_info_t event_infos[50];

static const char *event_info_type_strs[] = {
	"State",
	"Warning",
	"Interlock",
	"Error",
	"Parameter",
	"Message"
};

static const char *event_info_cause_strs[] = {
	"T&i\\TrigEnableCmd",
	"Tank\\OilPumpCmd",
	"Tank\\BpsVoltSet",
	"Ccps\\MainsOnCmd",
	"Ccps\\VoltSet",
	"Kly\\FpsCurrSet1",
	"Kly\\Sps1CurrSet",
	"Kly\\Sps2CurrSet",
	"Kly\\Sps3CurrSet",
	"Pdu\\FanOnCmd",
	"Kly\\HeaterDelay1",
	"Switch\\PlswthSet",
	"T&i\\LocPrfSet",
	"T&i\\PrfRead",
	"T&i\\FiberIntSts",
	"T&i\\HwCtrlExtIntlkSts",
	"T&i\\HwCtrlTrigEnableSts",
	"T&i\\HvEnaSts",
	"ExternalIntlk1",
	"ExternalIntlk2",
	"T&i\\Spare3",
	"T&i\\Spare4",
	"Pdu\\TmaxSts",
	"Ccps\\Ps1VoltRead",
	"Ccps\\Ps1VoltRead",
	"Ccps\\Ps1SumSts",
	"Ccps\\BleederSumSts",
	"Switch\\Su1SumSts",
	"Switch\\Su2SumSts",
	"Switch\\Su3SumSts",
	"Switch\\Su4SumSts",
	"Switch\\Su5SumSts",
	"Switch\\Su6SumSts",
	"Switch\\Su7SumSts",
	"Switch\\Su8SumSts",
	"Tank\\BiasPsOkSts",
	"Tank\\DigiCtRead",
	"Tank\\DigiCtRead",
	"Tank\\DigiCvdRead",
	"Tank\\DigiCvdRead",
	"Tank\\DigiFwhmRead",
	"Tank\\DigiFwhmRead",
	"Tank\\DigiSumSts",
	"Tank\\OilLevRead",
	"Tank\\OilLevRead",
	"Tank\\OilTempRead",
	"Tank\\OilTempRead",
	"Tank\\OilFilterSts",
	"Tank\\OilPumpInverterSts",
	"Tank\\OilPumpTempSts",
	"Kly\\FpsCurrRead",
	"Kly\\FpsCurrRead",
	"Kly\\FpsVoltRead",
	"Kly\\FpsVoltRead",
	"Kly\\Sps1CurrRead",
	"Kly\\Sps1CurrRead",
	"Kly\\Sps1VoltRead",
	"Kly\\Sps1VoltRead",
	"Kly\\Sps2CurrRead",
	"Kly\\Sps2CurrRead",
	"Kly\\Sps2VoltRead",
	"Kly\\Sps2VoltRead",
	"Kly\\Sps3CurrRead",
	"Kly\\Sps3CurrRead",
	"Kly\\Sps3VoltRead",
	"Kly\\Sps3VoltRead",
	"Kly\\SolPsSumSts",
	"Kly\\HwHvSumSts",
	"Kly\\HwStbSumSts",
	"Kly\\Ipc1CurrRead",
	"Kly\\Ipc1CurrRead",
	"Kly\\Ipc1VoltRead",
	"Kly\\Ipc1VoltRead",
	"Kly\\Ipc1VacOkSts",
	"Cool\\InletWaterTempRead",
	"Cool\\InletWaterTempRead",
	"Cool\\ColRtnTempRead",
	"Cool\\ColRtnTempRead",
	"Cool\\BodyRtnTempRead",
	"Cool\\BodyRtnTempRead",
	"Cool\\HwHvSumSts",
	"Cool\\HwStbSumSts",
	"Cool\\SolTempSwitchSts",
	"Cool\\FluidTrapSts",
	"Cool\\CcpsSuFlow1",
	"Cool\\CcpsSuFlow2",
	"Cool\\CcpsSuFlow3",
	"Cool\\BodWinFlow",
	"Cool\\CollectorFlow",
	"Cool\\SolenoidFlow",
	"Kly\\RfaSts",
	"Kly\\RF Drive Power Read",
	"Cool\\CollectorPower",
	"Cool\\BodyPower",
	"Kly\\RF Fwd Read",
	"Kly\\RF Fwd Read",
	"Kly\\RF Rfl Read",
	"Kly\\RF Rfl Read",
	"Kly\\RF Pulse Len Read",
	"Kly\\RF Pulse Len Read",
	"Kly\\RF DigiSumSts",
	"Kly\\RF VSWR Read",
	"GUIWatchdog",
	"IntComm",
	"ExtComm",
};

static const char *event_info_text_number_strs[6][101] = {
	{
	    "Error",
	    "Initialize",
	    "Off",
	    "BlkInterlock",
	    "BlkOffRequested",
	    "BlkOnRequested",
	    "Blk",
	    "FilInterlock",
	    "FilOffRequested",
	    "FilOnRequested",
	    "Fil",
	    "StbInterlock",
	    "StbOffRequested",
	    "StbOnRequested",
	    "Stb",
	    "HvInterlock",
	    "HvOffRequested",
	    "HvOnRequested",
	    "Hv",
	    "TrigInterlock",
	    "TrigOffRequested",
	    "TrigOnRequested",
	    "Trig",
		"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
		"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
		"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
		"", "", "", "", "", "", "", "", "", "", "", "", "", "",
	    "Disabled"
	}, {
		"Llim exceeded",
		"WLlim exceeded",
		"WHlim exceeded",
		"Hlim exceeded",
		"Input missing"
	}, {
		"Llim exceeded",
		"",
		"",
		"Hlim exceeded",
		"Input missing"
	}, {
        "Slave configuration error",
        "Slave error",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
        "GUI\\ScandiCat code",
        "GUI\\SelfTest code",
        "GUI\\UnitAi code",
        "GUI\\UnitAo code",
        "GUI\\UnitDi code",
        "GUI\\UnitDly code",
        "GUI\\UnitMultiAo code",
        "GUI\\UnitMultiDi code",
        "GUI\\Ads code",
        "GUI\\EventLogg code",
        "GUI\\Matrix code",
        "GUI\\Trend log code",
        "GUI\\Digitizer code",
        "GUI\\Config code"
	}, {
        "Hlim",
        "WHlim",
        "WLlim",
        "Llim",
        "Filter / ms",
        "uiScaleU",
        "uiScalL",
        "rScaleU",
        "rScaleL",
        "Set",
        "Set1",
        "Set2",
        "Invalid Set",
        "Invalid Set1",
        "Invalid Set2",
        "RampTime / s", 
        "OkDly / ms",
        "RampTime1 / s", 
        "RampTime2 /s",
        "OkDly1 /ms",
        "OkDly2 / ms",
        "RampStartSet",
        "Invalid RampStartSet",
        "RampOn",
        "TargetMatrix updated",
        "Hardware config saved",
        "CvdArcTripSet /kV",
        "CtArcTripSet /A",
        "CtIntegrateTripSet / A", 
        "CtIntegrateDelay /ns",
        "CtIntegrateSamples",
        "TrafoResistance / mOhm",
        "MaxDownTime / sec",
        "DownTimeFactor / sec"	
	}, {
		"AccessLevel: Remote",
		"AccessLevel: Operator",
		"AccessLevel: Admin",
		"AccessLevel: ScandiNova"
	}
};

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

static char *event_output_file = NULL;
static FILE *event_output_fd = NULL;
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

static void
event_data_to_str(uint32_t data, int type, char *out, size_t max_out_len_bytes)
{
	memset(out, 0, max_out_len_bytes);

	switch (type) {
	case 0: // no data
		break;
	case 1: // real
		assert(sizeof(float) == sizeof(uint32_t));
		snprintf(out, max_out_len_bytes - 1, "%f", *(float *) &data);
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

static long
handle_event_modify(aSubRecord *prec)
{
	struct tm *ts = NULL;
	uint64_t epoch_raw = 0;
	uint32_t upper_32 = 0;
	uint32_t lower_32 = 0;
	char timestamp[64];
	char timezone[8];
	event_index = *(int *) prec->a;
	event_info_t *event_info = &event_infos[event_index];

	// event timestamp

	if (event_index < 25) {
		upper_32 = ((uint32_t *) prec->c)[event_index * 2 + 1];
		lower_32 = ((uint32_t *) prec->c)[event_index * 2];
	} else {
		upper_32 = ((uint32_t *) prec->d)[(event_index - 25) * 2 + 1];
		lower_32 = ((uint32_t *) prec->d)[(event_index - 25) * 2];
	}

	epoch_raw = ((uint64_t) upper_32 << 32) | lower_32;

	if (epoch_raw == 0) {
		return 0;
	}

	event_info->epoch = epoch_raw / 10000000 - 11644473600;
	ts = localtime(&event_info->epoch);
	strftime(timestamp, sizeof(timestamp), "%a %Y-%m-%d %H:%M:%S", ts);
	strftime(timezone, sizeof(timezone), "%Z", ts);
	memset(event_info->timestamp, 0, sizeof(event_info->timestamp));
	snprintf(
		event_info->timestamp,
		sizeof(event_info->timestamp) - 1,
		"%s.%03d %s",
		timestamp,
		(int) ((epoch_raw % 10000000) / 10000),
		timezone);

	memset(prec->valc, 0, prec->novc);
	strncpy((char *) prec->valc, event_info->timestamp, prec->novc - 1);

	// event type

	event_info->type = ((uint16_t *) prec->e)[event_index];
	strncpy(event_info->type_str, event_info_type_strs[event_info->type], sizeof(event_info->type_str));
	
	memset(prec->vale, 0, prec->nove);
	strncpy((char *) prec->vale, event_info->type_str, prec->nove - 1);

	// event trigger id

	event_info->trigger = ((uint16_t *) prec->f)[event_index];
	*(long *) prec->valf = event_info->trigger;

	// event cause index

	event_info->cause_index = ((uint16_t *) prec->g)[event_index];
	strncpy(event_info->cause_str, event_info_cause_strs[event_info->cause_index], sizeof(event_info->cause_str));

	memset(prec->valg, 0, prec->novg);
	memset(prec->valk, 0, prec->novk);
	strncpy((char *) prec->valg, event_info->cause_str, prec->novg - 1);
	//strncpy((char *) prec->valk, event_info->cause_str, prec->novk - 1);

	// event text number

	event_info->text_number = ((uint16_t *) prec->h)[event_index];
	if (event_info->type == 0) {
		strncpy(event_info->text_number_str, event_info_text_number_strs[event_info->type][event_info->text_number + 1], sizeof(event_info->text_number_str));
	} else {
		strncpy(event_info->text_number_str, event_info_text_number_strs[event_info->type][event_info->text_number], sizeof(event_info->text_number_str));
	}
	memset(prec->valh, 0, prec->novh);
	strncpy((char *) prec->valh, event_info->text_number_str, prec->novh - 1);

	// event data type

	event_info->data_type = ((uint16_t *) prec->i)[event_index];
	strncpy(event_info->data_type_str, event_info_data_type_strs[event_info->data_type], sizeof(event_info->data_type_str));

	// event data

	event_info->data = ((uint32_t *) prec->j)[event_index];
	event_data_to_str(event_info->data, event_info->data_type, event_info->data_str, sizeof(event_info->data_str));


	if (event_info->type == 2) { // interlock
		memset(prec->vall, 0, prec->novl);
		memset(prec->valm, 0, prec->novm);
		memset(prec->valn, 0, prec->novn);
		memset(prec->valo, 0, prec->novo);
		memset(prec->valp, 0, prec->novp);

		strncpy((char *) prec->vall, event_info->type_str, prec->novl - 1);
		strncpy((char *) prec->valm, event_info->timestamp, prec->novm - 1);
		*(long *) prec->valn = event_info->trigger;
		strncpy((char *) prec->valo, event_info->cause_str, prec->novo - 1);
		//strncpy((char *) prec->valp, , prec->novp - 1);
	}

	if (event_output_fd != NULL) {
		fprintf(event_output_fd, "%d %s %d %s %s %s\n",
			event_index,
			event_info->type_str,
			event_info->trigger,
			event_info->cause_str,
			event_info->data_str,
			event_info->timestamp);
		fflush(event_output_fd);
	}

	return 0;
}
epicsRegisterFunction(handle_event_modify);


extern "C"
{

static const iocshArg event_log_configure_arg0 = {"filename", iocshArgString};
static const iocshArg *event_log_configure_args[] = {&event_log_configure_arg0};
static const iocshFuncDef event_log_func_def = {"set_event_output_file", 1, event_log_configure_args};
static void event_log_call_func(const iocshArgBuf *args)
{
	set_event_output_file(args[0].sval);
}

void event_log_register_commands(void)
{
	iocshRegister(&event_log_func_def, event_log_call_func);
}
epicsExportRegistrar(event_log_register_commands);

}
