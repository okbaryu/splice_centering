
var MasterDebug=0;

var REFRESH_IN_MILLISECONDS=1000;
var BMEMPERF_MAX_NUM_CPUS=8;
var passcount=0;
var UUID = ""; // used to allow multiple browsers to access the STB
var previous_height = 0;
var CgiTimeoutId=0;
var SpliceTimeoutId=0;
var CgiRetryTimeoutId=0;
var CgiCount=0; // number of times the cgi was called
var SetVariableCount=0; // number of times setVariable() is called
var ResponseCount=0; // number of times serverHttpResponse() is called
var objdebug = 0;
var epochSeconds = 0;
var tzOffset = 0;
var localtime = 0; // usually new Date() but could also be time read in from recording file
var localdatetime = "";
var userAgent = 0;
var gNumCpus = 0;
var gCpuData = [0,0,0,0,0,0,0,0];
var gCpuFreq = [0,0,0,0,0,0,0,0];
var gIrqLatestData = [0,0,0,0,0,0,0,0];
var gIrqMaxData = [0,0,0,0,0,0,0,0];
var bCpuDataHeightSet = 0;
var bIrqDataHeightSet = 0;
var NetBytesPrev =[[0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0]]; // up to 10 network interfaces with Rx and Tx for each of the 10
var NetBytesCummulative =[[0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0]]; // up to 10 network interfaces with Rx and Tx for each of the 10
var NetBytesRx10SecondsCount=[0,0,0,0,0,0,0,0,0,0];
var NetBytesRx10Seconds =[[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0]
,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0]];
var NetBytesTx10SecondsCount=[0,0,0,0,0,0,0,0,0,0];
var NetBytesTx10Seconds =[[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0]
,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0]];
var NetBytesSeconds = 0; // net bytes are accumulated for as long as the browser is on this page; this value is used to compute the average Mbps
var NetGfxCheckbox=[0,0,0,0,0,0,0,0,0,0]; // up to 10 network interfaces ... indicates whether checkbox_netifgfx is checked or not
var NetGfxDivisor= [[1,1,1,1,1,1,1,1,1,1], [1,1,1,1,1,1,1,1,1,1]]; // divisor will increase/decrease relative to network bytes received per second
var NetGfxBytes= [["","","","","","","","","",""], ["","","","","","","","","",""]]; // 10 network interfaces ... also indexed by Rx/Tx
var RxTxStr = ["rx","tx"];
var POINTS_PER_GRAPH = 250;
var GetCpuInfo = {Value: 0};
var GetSysInfo = {Value: 0};
var GetTime= {Value: 0};
var GetMemory = {Value: 0};
var GetIrqInfo = {Value: 0};
var GetNetStats =  {Value: 0};
var GetNetStatsInit = true; // set to true when user checks the checkbox; then set to false
var GetWifiStats =  {Value: 0, Init:0, SecondsEnabled:0 };
var GetWifiStatsCountdown = 0;
var GetWifiScan =  {Value: 0, State:0, MaxNumAps:1, ServerIdx:0 };
var GetWifiScanState = { UNINIT:0 ,SCANNING:1 };
var GetWifiAmpduGraph =  {Value: 0, FirstTime:0 };
var HIDE = false;
var SHOW = true;
var GetHeapStats =  {Value: 0};
var GetSataUsb =  {Value: 0, Stop:0 };
var GetProfiling =  {Value: 0};
var GetContextSwitch = {Value: 0};
var GetPerfTop =  {Value: 0};
var GetLinuxTop =  {Value: 0, Stop:0 };
var GetPerfCache =  {Value: 0, Duration:0, StartReportNow:false };
var GetPerfCacheResults =  {Value: false };
var GetPerfCacheCountdown = 0;
var GetPerfDeep =  {Value: 0, Duration:0, StartReportNow:0 };
var GetPerfDeepResults =  {Value: false };
var GetPerfDeepCountdown = 0;
var GetPerfFlame =  {State:0, Value: 0, Stop:0 };
var GetPerfFlameResults =  {Value: false };
var GetPerfFlameRecordingSeconds = 0;
var GetPerfFlamePidCount = 0;
var ActStatus = {ACTDIR:0, ACTSTROKE:0, ACTSPEED:0, ACTLLIMIT:0, ACTRLIMIT:0, ACTORGMSB:0, ACTORGLSB:0};
var actCurPos = 0;
var actPrevPos = 0;

//  GetPerfFlameState needs to match the enum Bsysperf_PerfFlame_State in bsysperf.c
var GetPerfFlameState = { UNINIT:0, INIT:1, IDLE:2, START:3, RECORDING:4, STOP:5, CREATESCRIPTOUT:6, GETSVG:7, DELETEOUTFILE:8 };

var GetPerfFlameSvgCount = 1;
var PerfRecordUuid = "";
var FlameWindow = 0;
var validResponseReceived = 0;
var GlobalDebug = 0;
var CountdownSeconds = 0;
var PerfCacheCountdownSeconds = 0;
var PERF_STAT_DURATION = 3;
var gEvent = 0;
var PerfError = true; // set to true of perf tool cannot be found
var ChangeCpuState = 0; // used to enable or disable a specific CPU
var urlSentPreviously = "";
var urlSentSuccessfully = 0;
var urlSentRetryCount = 0;
var urlSentRetryCountAfterSuccess = 0;
var eol = "\n"; // end of line character for Firefox, Chrome, Safari (not for ie7 ... use <br>)
var cpuUsageLongAverage = [0,0,0,0,0,0,0,0,0,0]; // 10-second average CPU utilization
var CPU_USAGE_LONG_AVERAGE_MAX = 5; // 10-second window
var cpuUsageLongAverageIdx = 0; // index into 10-second average CPU utilization array (cpuUsageLongAverage)
var cpuUsageLongAverageCount = 0; // number of 10-second average CPU utilization array (cpuUsageLongAverage)
var iperfStateEnum = { UNINIT:0, INIT:1, RUNNING:2, STOP:3 };
var iperfStateClient = iperfStateEnum.UNINIT;
var iperfStateServer = iperfStateEnum.UNINIT;
var iperfStateServerRetry = 0; // the 1st time after reboot, the start of the server happens so slowly, the pid comes back 0 which causes JS to think the server died
var iperfInit = false;
var iperfStartTimeClient = 0;
var iperfStartTimeServer = 0;
var iperfTimeoutClient = 0;
var iperfPidClient = 0;
var iperfPidServer = 0;
var iperfRunningClient = "";
var iperfRunningServer = "";
var iperfClientServerRow = 0;
var iperfClientRate = "";
var DvfsControl =  {Value: 0};
var RecordControl =  {Value: 0};
var RecordFileSize = 0;
var RecordFileContents = "";
var RecordFileContents2 = ""; // used by asynchronous reader.onload() function in readText() function
var RecordFileContentsArray = new Array();
var RecordTimeStart = 0;
var PlaybackControl =  {Value: 0}; // 0=>idle, 1=>read next entry
var PlaybackTimeSeconds = 0;
var PlaybackFileContentsArrayIdx = 0;
var PlaybackTimeoutId=0;
var PlaybackPLATFORM = "";
var PlaybackVARIANT = "";
var PlaybackPLATVER = "";
var PlaybackVERSION = "";
var PlaybackUNAME = "";
var PlaybackBOLTVER= "";
var PlaybackCPUPERCENTS = "";
var PlaybacknetStatsInit = "";
var PlaybackSTBTIME = "";
var PlaybackSTBTIMEsaved = ""; // used to prevent duplicate records with the same time value
var PlaybackElementsDisabled = ""; // set to list of element ids that are disabled for playback
var sRecordFilename = "";
var gTextFile = null;
var GovernorSettingNow = 0;
var GovernorSettingPrev = 0;

Number.prototype.padZero= function(len){
	var s= String(this), c= '0';
	len= len || 2;
	while (s.length < len) s= c + s;
	return s;
}

function randomFromTo(from, to)
{
	return Math.floor(Math.random() * (to - from + 1) + from);
}

function OneSecond ()
{
	var debug=0;

	//alert("OneSecond");
	MyLoad();

	// if previous timeout is already pending, cancel it
	clearTimeoutOneSecond( "OneSecond");

	//console.log( "OneSecond: calling setTimeout()");
	CgiTimeoutId = setTimeout ('OneSecond()', REFRESH_IN_MILLISECONDS );
}

function FiveSecond ()
{
	var url = "";

	url = "/cgi/splice.cgi?";
	url += "&isAlive=" + 1;
	sendCgiRequestDoItNow(url);

	clearTimeoutFiveSecond();
	SpliceTimeoutId = setTimeout('FiveSecond()', REFRESH_IN_MILLISECONDS * 10);
}

function getNumEntries ( arrayname )
{
	var num_entries = arrayname.length;
	if (userAgent.indexOf("MSIE") >= 0 ) {
		//num_entries++; // for ie9, length is one less than firefox, chrome, safari
	}
	//alert("1 array(" + arrayname + "); num points" + num_entries );

	return num_entries;
}

function hideOrShow ( elementid, trueOrFalse )
{
	var obj=document.getElementById(elementid);

	if ( obj ) {
		if (trueOrFalse) {
			//if (elementid.indexOf('wifi') ) alert("hideOrShow: " + elementid + "; starting to SHOW");
			obj.style.display = '';
			obj.style.visibility = '';
			//alert("hideOrShow: " + elementid + "; SHOW");
		} else {
			//if (elementid.indexOf('wifi') ) alert("hideOrShow: " + elementid + "; starting to HIDE");
			obj.style.display = 'none';
			obj.style.visibility = 'hidden';
			//console.log("hideOrShow: " + elementid + "; HIDE");
		}
	}

	return true;
}

function hideElement ( elementid )
{
	elemobj = document.getElementById(elementid);
	if (elemobj) {
		//console.log("hiding element " + elementid );
		elemobj.style.visibility = "hidden";
	}
}

function showRow ( rownum )
{
	var rowid;
	var rowobj;

	rowid = "row0" + rownum + "a";
	rowobj = document.getElementById(rowid);
	if (rownum%2 == 1 && rowobj && rownum < gNumCpus ) {
		var even_number_of_cpus = gNumCpus%2;
		//alert("rowobj " + rowid + "; obj " + rowobj + "; gNumCpus " + gNumCpus + "even_number " + even_number_of_cpus );

		// if we have an odd number of cpus, hide the right hand column
		if (even_number_of_cpus == 0) {
			rowid = "rightcol0" + rownum + "a";
			hideElement(rowid);
			rowid = "rightcol0" + rownum + "b";
			hideElement(rowid);
			rowid = "rightcol0" + rownum + "c";
			hideElement(rowid);
		}

		rowobj.style.visibility = "";
		rowid = "row0" + rownum + "b";
		rowobj = document.getElementById(rowid);
		rowobj.style.visibility = "";
		rowid = "row0" + rownum + "c";
		rowobj = document.getElementById(rowid);
		rowobj.style.visibility = "";
	}
	//alert("showRow done");
}

function setButtonDisabled ( targetId, newState )
{
	//console.log("setButtonDisabled  targetId (" + targetId + ") ... newState (" + newState + ")" );
	var objButton = document.getElementById( targetId );
	if (objButton) {
		objButton.disabled = newState;
	} else {
		console.log( "could not find element ... " + targetId );
	}

	return true;
}

function SetCheckboxDisabled ( checkboxName, objStatus )
{
	var obj = document.getElementById( checkboxName );
	if (obj ) {
		//if (checkboxName =="checkboxPerfCache"  ) { alert("SetCheckboxDisabled ( " + checkboxName + ") = " + objStatus + "; PerfError=" + PerfError ); }
		if ( (checkboxName =="checkboxPerfTop" || checkboxName == "checkboxPerfDeep" || checkboxName == "checkboxPerfCache" ) && ( PerfError == true ) ) {
			// do not allow changes to Perf checkboxes if kernel is not compiled to include perf tools
			obj.disabled = true;
		} else {
			obj.disabled = objStatus;
		}
		//alert("settings (" + checkboxName + ") status (" + objStatus + ") to value (" + obj.disabled + ")" );
	}
}

function GetCheckboxStatus ( checkboxName )
{
	var rc = false;
	var obj = document.getElementById( checkboxName );
	if (obj ) {
		rc = obj.checked;
		//alert("settings (" + checkboxName + ") status (" + objStatus + ") to value (" + objStatus.Value + ")" );
	}

	return rc;
}

function SetCheckboxStatus ( checkboxName, objStatus )
{
	var obj = document.getElementById( checkboxName );
	if (obj ) {
		//if (checkboxName =="checkboxPerfCache"  ) { alert("SetCheckboxStatus ( " + checkboxName + ") = " + objStatus.Value + "; PerfError=" + PerfError ); }
		if ( (checkboxName =="checkboxPerfTop" || checkboxName == "checkboxPerfDeep" || checkboxName == "checkboxPerfCache" ) && ( PerfError == true ) ) {
			// do not allow changes to Perf checkboxes if kernel is not compiled to include perf tools
			obj.checked = false;
		} else {
			obj.checked = objStatus.Value;
		}
		//alert("SetCheckboxStatus (" + checkboxName + ") to value (" + objStatus.Value + ")" );
	} else {
		//alert("SetCheckboxStatus (" + checkboxName + ") is unknown" );
	}
}

function SetInternalCheckboxStatus ( checkboxName, objStatus )
{
	var obj = document.getElementById( checkboxName );
	if (obj ) {
		objStatus.Value = obj.checked;
		//alert("SetInternalCheckboxStatus (" + checkboxName + ") to value (" + objStatus.Value + ")" );
	}
}

function SvgClickHandler (event)
{
	var target = event.target || event.srcElement;
	var id = target.id;
	//alert("svgClickHandler: id (" + id + ");" );
	MyClick(event);
}

/*
 * This function is called to update the CPU utilization graph as well as the Network Interface graphs.
 */
function AddSvgPoints ( svgobj, newValue )
{
	var coords = svgobj.getAttribute('points' );
	var minYcoord = 100;

	if (coords == null) {
		coords = "0," + newValue + " ";
	}

	if (coords) {
		var coords2 = rtrim(coords);
		var splits = coords2.split(' ');
		var newcoords = "";
		var starting_idx = 0;
		var num_entries = getNumEntries(splits);

		if ( num_entries < POINTS_PER_GRAPH ) {
			starting_idx = 0;
		} else {
			starting_idx = 1;// skip the first element; it is dropping off the left side
		}
		for (idx=starting_idx; idx < num_entries; idx++ ) {
			var justone = splits[idx].split(',');
			newcoords += idx*2 + "," + justone[1] + " " ;
			minYcoord = Math.min ( minYcoord, justone[1] );
		}

		// add new last point to the far right
		newcoords += idx*2 + "," + newValue + " " ;
		minYcoord = Math.min ( minYcoord, newValue );

		svgobj.setAttribute('points', newcoords );
	}

	return minYcoord;
}

function AppendPolylineXY ( polylinenumber, newValue )
{
	var lineid = "polyline0" + polylinenumber;
	var svgobj = document.getElementById(lineid);

	// alert("polylinenumber " + polylinenumber + "; gNumCpus " + gNumCpus + "; svgobj " + svgobj + "; lineid " + lineid );
	if (svgobj && gNumCpus && polylinenumber<(gNumCpus+1) ) { // added one for limegreen 5-second average line
		// coords example: 0,100 2,85 5,95 6,95 (i.e. x,y where the y value is pixels "below" the top of the graph; 100 pixels means 0 percent
		if (polylinenumber < (gNumCpus+1) ) { // if we are processing the cpu poly lines
			if ( newValue == 255 ) { // if the cpu is inactive
			} else {
				AddSvgPoints ( svgobj, newValue );
			}
		}
	} else {
		//alert ("lineid " + lineid + " does not exist");
	}
}

// There is a bug that if the CPU row is not already checked in the HTML file, the blank space for the CPU rows
// will still be seen. Created this delayed function to hide the CPU row if the checkbox is unchecked.
function Wifi3DDelayed()
{
	var obj = document.getElementById('checkboxcpus');
	if ( obj && obj.checked == false) {
		hideOrShow("row_cpus", HIDE );
	}
	hideOrShow("row_netgfx_all", HIDE );
}

function uuid()
{
	function s4() {
		return Math.floor((1 + Math.random()) * 0x10000)
			.toString(16)
			.substring(1);
	}
	//return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
	return s4() + s4() + s4() + s4() + s4() + s4() + s4() + s4();
}

function UpdateCpuGraphs()
{
	var objCheckboxCpus = document.getElementById("checkboxcpus");
	if (objCheckboxCpus && objCheckboxCpus.checked ) {
		for (var polylinenumber=1; polylinenumber<(gNumCpus-1)+1 ; polylinenumber++) { //once for all CPUs
			AppendPolylineXY ( polylinenumber, gCpuData[polylinenumber-1] );
		}
	}
}

function MyLoad()
{
	userAgent = navigator.userAgent;
	//alert("MyLoad: browser (" + userAgent + "); passcount " + passcount);
	var idx;
	var transformAttr = "";
	var polylinenumber;
	var rowid;
	var lineid;
	var rowobj;
	var svgobj;

	objdebug = document.getElementById("debugoutputbox");

	//alert("passcount==" + passcount + "; gNumCpus " + gNumCpus );
	if ( passcount == 0 ) {
		var local = new Date();

		UUID = uuid();

		var obj2 = document.getElementById("checkboxwifi");
		if ( obj2 ) {
			obj2.checked = false;
			GetWifiStats.Init = false;
			hideOrShow("row_wifi_stats", HIDE );
			hideOrShow("row_wifi_ampdu", HIDE );
			GetWifiAmpduGraph.Value = 0;
			setTimeout ('Wifi3DDelayed()', 100 );
		}

		epochSeconds = Math.floor(local.getTime() / 1000);
		//alert("local Date " + epochSeconds );
		tzOffset = local.getTimezoneOffset();
		//alert("TZ offset " + local.getTimezoneOffset() );
		localdatetime = (local.getUTCMonth()+1).padZero() + local.getUTCDate().padZero() + local.getUTCHours().padZero() + local.getUTCMinutes().padZero() +
			local.getUTCFullYear() + "." + local.getUTCSeconds().padZero();

		var newpoints = "";
		var newheight = 0;
		for (polylinenumber=1; polylinenumber<(gNumCpus-1)+1 ; polylinenumber++) { //once for all CPUs
			newpoints = "20,100 "; // 100% idle
			lineid = "polyline0" + polylinenumber;
			//alert("lineid " + lineid );
			svgobj = document.getElementById(lineid);
			if (svgobj) {
				var rect = svgobj.getBoundingClientRect();

				//alert("lineid " + lineid + "; left " + rect.left + "; right " + rect.right + "; top " + rect.top + ";btm " + rect.bottom );
				for (idx=2; idx<1-1; idx+=2) {
					newheight = randomFromTo(0,100);
					newpoints += idx + "," + newheight + " ";
				}
				//alert("for " + lineid + ", 1st newpoints (" + newpoints + ")" + svgobj );
				svgobj.setAttribute('points', newpoints );
			}
		}

		SetInternalCheckboxStatus ( "checkboxcpus", GetCpuInfo );
		SetInternalCheckboxStatus ( "checkboxmemory", GetMemory );
		SetInternalCheckboxStatus ( "checkboxnets", GetNetStats );
		SetInternalCheckboxStatus ( "checkboxwifi", GetWifiStats );
		SetInternalCheckboxStatus ( "checkboxirqs", GetIrqInfo );
		SetInternalCheckboxStatus ( "checkboxprofiling", GetProfiling );
		SetInternalCheckboxStatus ( "checkboxPerfFlame", GetPerfFlame);

		iperfInit = GetNetStats.Value;

		GetSataUsb.Stop = 1; // tell the server to stop any SATA/USB data collection that may have been started earlier

		hideOrShow("row_DvfsControl", HIDE );
		hideOrShow("row_Record", HIDE );

		var queryString = "";
		if( window.location.search.length > 1) {
			queryString = window.location.search.substring (1);
			var elements = queryString.split("&");
			var keyValues = {};
			for(var i in elements) {
				var key = elements[i].split("=");
				if (key.length > 1) {
					keyValues[decodeURIComponent(key[0].replace(/\+/g, " "))] = decodeURIComponent(key[1].replace(/\+/g, " "));
				}
			}
			//console.log("keyValue RecordFile is (" + keyValues['RecordFile'] + ")" );
			//console.log("keyValue RecordButton is (" + keyValues['RecordButton'] + ")" );
			if ( keyValues['RecordButton'] == 1 ) {
				//console.log("detected RecordButton is 1");
				var obj = document.getElementById("checkboxRecord" );
				if ( obj ) {
					obj.checked = true;
					hideOrShow("row_Record", SHOW );
				}
			}
		}

		//var objRecordSaveFile = document.getElementById("RecordSaveFile");
		//objRecordSaveFile.addEventListener('click', RecordSaveFileListener, false);

		//var objExportCpuStats = document.getElementById("buttonExportCpuStats");
		//objExportCpuStats.addEventListener('click', ExportCpuStatsListener, false);

		//var objExportNetStats = document.getElementById("buttonExportNetStats");
		//objExportNetStats.addEventListener('click', ExportNetStatsListener, false);

		//console.log( "MyLoad: calling sendCgiRequest()");
		CgiTimeoutId = setTimeout ('OneSecond()', REFRESH_IN_MILLISECONDS );
		SpliceTimeoutId = setTimeout ('FiveSecond()', REFRESH_IN_MILLISECONDS * 5);

		//alert("pass 0 done");
	} else {
		UpdateCpuGraphs();
	}
	//alert("polyline01 points (" + document.getElementById('polyline01').getAttribute('points') + ")" );
	previous_height += 10;
	//alert("prev_hgt " + previous_height);
	if ( previous_height > 100) previous_height = 0;

	//var svgid = 'svg' + GetPerfFlameSvgCount.padZero(4);
	var svgid = 'svg001';
	svgid = 'svg' + GetPerfFlameSvgCount.padZero(4);
	//alert("MyLoad - svgid:" + svgid );
	svgobj = document.getElementById( svgid );
	if (svgobj) {
		svgobj.addEventListener("click", SvgClickHandler, false);
	}
	passcount++;

	sendCgiRequest();

	//alert("MyLoad: end passcount==" + passcount);
}

function randomFromTo(from, to)
{
	return Math.floor(Math.random() * (to - from + 1) + from);
}

function rtrim(stringToTrim) { return stringToTrim.replace(/\s+$/,"");}

function MyChange(event)
{
	var target = event.target || event.srcElement;
	var id = target.id;
	var value=0;
	var obj=document.getElementById(id);
	if (obj) {
		value = obj.value;
	}
	//alert("MyChange: value " + value);
	setVariable(id);
}
function AdjustRectangleToTextWidth ( id )
{
	var obj=document.getElementById(id);
	if (obj) {
		var pointsStr = obj.getAttribute("points");
		var pointsStrNew = "";

		//alert("points (" + pointsStr + ")" );

		// points (190,10  320,10  320,60  190,60 )
		var points = pointsStr.split( " " );
		pointsStrNew = points[0] + " ";
		//alert("points1 (" + pointsStrNew + ")" );

		var xypair = points[1].split( ",");
		pointsStrNew += Number(xypair[0]) + Number(400);
		pointsStrNew += "," + xypair[1] + " ";
		//alert("points2 (" + pointsStrNew + ")" );

		xypair = points[2].split( ",");
		pointsStrNew += Number(xypair[0]) + Number(400);
		pointsStrNew += "," + xypair[1] + " ";
		//alert("points3 (" + pointsStrNew + ")" );

		pointsStrNew += points[3];
		//alert("points4 (" + pointsStrNew + ")" );

		obj.setAttribute("points", pointsStrNew );
	} else {
		alert("getElementById(" + id + ") failed");
	}
}

function SliderCB(event, ui)
{
	var url = "";
	var org_val = document.getElementById("origin_val").value;

	if(ui.value != org_val)
	{
		url = "/cgi/splice.cgi?";
		url += "&actPos=" + ui.value;
		sendCgiRequestDoItNow(url);
	}
}

function readCmdPLC()
{
	var url = "";

	url = "/cgi/splice.cgi?";
	url += "&readCmdPLC=" + 1;
	sendCgiRequestDoItNow(url);
}

function GetACTStatus()
{
	var url = "";

	url = "/cgi/splice.cgi?";
	url += "&getActStatus=" + 1;
	sendCgiRequestDoItNow(url);
}

function GetIPfromFile()
{
	var rawFile = new XMLHttpRequest(); // XMLHttpRequest (often abbreviated as XHR) is a browser object accessible in JavaScript that provides data in XML, JSON, but also HTML format, or even a simple text using HTTP requests.
	rawFile.open("GET", "ipaddr", false); // open with method GET the file with the link file ,  false (synchronous)
	rawFile.onreadystatechange = function ()
	{
		if(rawFile.readyState === 4) // readyState = 4: request finished and response is ready
		{
			if(rawFile.status === 200) // status 200: "OK"
			{
				var allText = rawFile.responseText; //  Returns the response data as a string
				document.getElementById("ip_addr").value = allText;
			}
		}
	}
	rawFile.send(null);
}

function SetACTStatus()
{
	ActStatus.ACTDIR = document.getElementById("direction_val").value;
	ActStatus.ACTSTROKE = document.getElementById("stroke_val").value;
	ActStatus.ACTSPEED = document.getElementById("speed_val").value;
	ActStatus.ACTLLIMIT = document.getElementById("left_limit_val").value / 100;
	ActStatus.ACTRLIMIT = document.getElementById("right_limit_val").value / 100;
	ActStatus.ACTORG = document.getElementById("origin_val").value;

	//alert(ActStatus.ACTLLIMIT + ":" + ActStatus.ACTRLIMIT);
	var url = "";

	url = "/cgi/splice.cgi?";
	url += "&setActStatus=" + 1;
	url += "&ACTDIR=" + ActStatus.ACTDIR;
	url += "&ACTSTROKE=" + ActStatus.ACTSTROKE;
	url += "&ACTSPEED=" + ActStatus.ACTSPEED;
	url += "&ACTLLIMIT=" + ActStatus.ACTLLIMIT;
	url += "&ACTRLIMIT=" + ActStatus.ACTRLIMIT;
	url += "&ACTORG=" + ActStatus.ACTORG;

	sendCgiRequestDoItNow(url);
}

function MyClick(event)
{
    var target = event.target || event.srcElement;
    var id = target.id;
    //alert("MyClick: id (" + id + ");" );


    gEvent = event;

    setVariable(id);
}

function Act_left_button(event)
{
	var value = $( "#Slider1" ).slider( "option", "value" );
	var llimit = document.getElementById("left_limit_val").value / 100 * -1;

	if(value - 1 < llimit)
	{
		alert("Invalid actuator position! " + value);
		$( "#Slider1" ).slider( "option", "value", llimit );
		$("#calibar_val").val(llimit);
	}
	else
	{
		$( "#Slider1" ).slider( "option", "value", value - 1 );
		$("#calibar_val").val(value - 1);
	}
}

function Act_right_button(event)
{
	var value = $( "#Slider1" ).slider( "option", "value" );
	var rlimit = document.getElementById("right_limit_val").value / 100;

	if(value + 1 > rlimit)
	{
		alert("Invalid actuator position! " + value);
		$( "#Slider1" ).slider( "option", "value", rlimit );
		$("#calibar_val").val(rlimit);
	}
	else
	{
		$( "#Slider1" ).slider( "option", "value", value + 1 );
		$("#calibar_val").val(value + 1);
	}
}

function Act_origin_button(event)
{
	var url = "";
	var value = document.getElementById("origin_val").value;
	$( "#Slider1" ).slider( "option", "value", value / 100 );
	$("#calibar_val").val(value / 100);

	url = "/cgi/splice.cgi?";
	url += "&actPosOrg=" + 1;
	sendCgiRequestDoItNow(url);
}

function DisableCheckboxes ( newValue )
{
    var elements = document.getElementsByTagName("input");
    //alert("elements (" + elements + ")" );
    for (var i = 0; i < elements.length; i++ ) {
        //alert("element.type (" + elements[i].type + ")" );
        if (elements[i].type === "checkbox" ) {
            var name = elements[i].id;
            var ischecked = elements[i].checked; //check if checked
            //alert("checkbox: " + name + "; ischecked " + ischecked );
            elements[i].disabled = newValue;
        }
    }
}

function GetInputValue ( fieldName )
{
    var rc=0;
    var obj=document.getElementById( fieldName );
    if (obj) {
        rc = obj.value;
    }
    return rc;
}

function SetInputValue ( fieldName, newValue )
{
    var rc=0;
    //alert("SetInput: name " + fieldName + "; newValue " + newValue );
    var obj=document.getElementById( fieldName );
    if (obj) {
        obj.value = newValue;
    }
    return rc;
}

function checkboxPerfDeepDoit( fieldValue )
{
    if ( fieldValue == true ) {
        // if previous timeout is already pending, cancel it
        clearTimeoutOneSecond( "checkboxPerfDeepDoit");

        //alert("checkboxPerfDeepDoit: value " + fieldValue + "; GetPerfDeep.Value (" + GetPerfDeep.Value + ")" );
        SetInternalCheckboxStatus ( "checkboxPerfDeep", GetPerfDeep);
        GetPerfDeep.Duration = GetInputValue ( "PerfDeepDuration" );
        //alert("checkboxPerfDeepDoit done");
    }

    SetCheckboxDisabled ( "checkboxPerfTop", fieldValue );
    SetCheckboxDisabled ( "checkboxPerfCache", fieldValue );
}

function checkboxPerfCacheDoit( fieldValue )
{
    if ( fieldValue == true ) {
        clearTimeoutOneSecond( "checkboxPerfCacheDoit");

        //alert("checkboxPerfCacheDoit: value " + fieldValue + "; GetPerfCache.Value (" + GetPerfCache.Value + ")" );
        SetInternalCheckboxStatus ( "checkboxPerfCache", GetPerfCache);
        GetPerfCache.Duration = GetInputValue ( "PerfCacheDuration" );
        //alert("checkboxPerfCacheDoit done");
    }

    SetCheckboxDisabled ( "checkboxPerfTop", fieldValue );
    SetCheckboxDisabled ( "checkboxPerfDeep", fieldValue );
}

function GetPerfFlameSetState( newstate )
{
    //alert("GetPerfFlameSetState:" + newstate );
    GetPerfFlame.State = newstate;
    var objstate = document.getElementById('PerfFlameState');
    if (objstate) {
        if (newstate == GetPerfFlameState.INIT) {
            objstate.innerHTML = "INIT";
        } else if (newstate == GetPerfFlameState.IDLE) {
            objstate.innerHTML = "IDLE";
        } else if (newstate == GetPerfFlameState.START) {
            objstate.innerHTML = "START";
        } else if (newstate == GetPerfFlameState.RECORDING) {
            objstate.innerHTML = "RECORDING";
        } else if (newstate == GetPerfFlameState.STOP) {
            objstate.innerHTML = "STOP";
        } else if (newstate == GetPerfFlameState.CREATESCRIPTOUT) {
            objstate.innerHTML = "CREATESCRIPTOUT";
        } else if (newstate == GetPerfFlameState.GETSVG) {
            objstate.innerHTML = "GETSVG";
        } else if (newstate == GetPerfFlameState.DELETEOUTFILE) {
            objstate.innerHTML = "DELETEOUTFILE";
        } else {
            objstate.innerHTML = "UNKNOWN";
        }
    }
}

function GetPerfFlameSetStop()
{
    GetPerfFlameSetState( GetPerfFlameState.STOP ); // Stop recording and send back report
    var objButton = document.getElementById('PerfFlameStartStop');
    if (objButton) {
        objButton.value = "Start";
    }
    setButtonDisabled( 'checkboxPerfFlame', false );
}

function ClearOutHtml ( tagName )
{
    var obj = document.getElementById( tagName );
    if (obj) {
        obj.innerHTML = "";
    }

    return true;
}

function AddToHtml ( tagName, newHtml )
{
    var obj = document.getElementById( tagName );
    if (obj) {
        obj.innerHTML += newHtml;
    }

    return true;
}

var checkboxWifiAmpduGraph1stPass = true;

function checkboxWifiAmpduGraphDoit ( fieldValue )
{
    SetInternalCheckboxStatus ( "checkboxWifiAmpduGraph", GetWifiAmpduGraph);
    if ( fieldValue == false ) { // we unchecked the box ... stop recording
        GetWifiAmpduGraph.Value = 0;
        hideOrShow("row_wifi_ampdu", HIDE );
    } else {
        if ( userAgent.indexOf("Trident") > 0 ) { // agents like Trident do not render the SVG
            alert("The AMPDU graph using SVG elements does not work on this browser!     (Try Chrome, Firefox, or Safari.)");
            var obj2 = document.getElementById("checkboxWifiAmpduGraph");
            if (obj2) {
                obj2.disabled = true;
                obj2.checked = false;
            }
        } else {
            GetWifiAmpduGraph.Value = 1;
            if ( GetWifiAmpduGraph.FirstTime == 0) { // if this is the first time, tell bsysperf_server to begin collecting data
                GetWifiAmpduGraph.FirstTime = 1;
            }
        }
        if (checkboxWifiAmpduGraph1stPass) {
            checkboxWifiAmpduGraph1stPass = false;
            for(sidx=0; sidx<2; sidx++) { // rotate both SVG elements to the left a bit
                for(var i=0; i<6; i++) {
                    ChangeViewer(0,-5);
                }
                // shift both SVG elements up a bit
                for(var i=0; i<1; i++) { // 2016-07-14 ... was 3
                    Shift(0,-20);
                }
                S[sidx].ZoomAll = 26;
            }
            sidx=0;
        }
    }
}

function checkboxwifiDoit( fieldValue )
{
    SetInternalCheckboxStatus ( "checkboxwifi", GetWifiStats );
    hideOrShow("row_wifi_stats", fieldValue);
    if (fieldValue == false) {
        hideOrShow("row_wifi_ampdu", fieldValue);
    }
    GetWifiStats.Init = fieldValue;
}

function ClearNetGfxGraph ( idx )
{
    for ( var RxTxIndex=0; RxTxIndex<2; RxTxIndex++) {
        var objline = document.getElementById( 'polyline_netgfx_' + RxTxStr[RxTxIndex] + '_' + idx );
        if ( objline ) {
            objline.setAttribute('points', '' ); // clear out the polyline coordinates for each of 10 network interfaces ... two graphs each (Rx/Tx)
        }
        NetGfxDivisor[RxTxIndex][idx] = 1;
        NetGfxBytes[RxTxIndex][idx] = "";
    }
}

function checkboxSelected ( fieldName, fieldValue )
{
    var debug=0;
    if (debug==1) alert("checkboxSelected: " + fieldName + "; value " + fieldValue );
    var obj=document.getElementById(fieldName);
    if ( obj ) obj.checked = fieldValue;

    if (fieldName == "checkboxcpus" ) {
        SetInternalCheckboxStatus ( "checkboxcpus", GetCpuInfo );
        hideOrShow("row_cpus", fieldValue);
    } else if (fieldName == "checkboxnets" ) {
        SetInternalCheckboxStatus ( "checkboxnets", GetNetStats );
        hideOrShow("row_net_stats", fieldValue);
        iperfInit = fieldValue;
        GetNetStatsInit = fieldValue;

        // hide all ten of the histogram rows
        for ( var idx=0; idx<10; idx++) {
            hideOrShow("row_netgfxsvg" + idx, HIDE );
            hideOrShow("row_netgfxtxt" + idx, HIDE );
            NetGfxCheckbox[idx] = 0;

            ClearNetGfxGraph ( idx );
        }
    } else if (fieldName == "checkboxwifi" ) {
        checkboxwifiDoit( fieldValue );
    } else if (fieldName == "checkboxirqs" ) {
        SetInternalCheckboxStatus ( "checkboxirqs", GetIrqInfo );
        hideOrShow("row_irqs", fieldValue);
    } else if (fieldName == "checkboxheapstats" ) {
        SetInternalCheckboxStatus ( "checkboxheapstats", GetHeapStats );
    } else if (fieldName == "checkboxsatausb" ) {
        SetInternalCheckboxStatus ( "checkboxsatausb", GetSataUsb );
        if (GetSataUsb.Value == 0) { // if user un-checked the box, tell server to stop collecting data
            GetSataUsb.Stop = 1;
        }
    } else if (fieldName == "checkboxprofiling" ) {
        GetProfiling.Value = fieldValue;

        // when Profiling checkbox is first checked, default PerfTop checkbox to on also
        GetPerfTop.Value = fieldValue;

        // when hiding profiling, tell server to stop all data collection that might be going on
        if (fieldValue == false) {
            GetPerfTop.Value = false;
            GetLinuxTop.Value = false;
            GetLinuxTop.Stop = 1;
            GetPerfDeep.StartReportNow = false;
            GetPerfDeep.Value = 0;
            GetContextSwitch.Value = false;
            //AddToDebugOutput ( "checkboxprofiling: PerfCache.Start (" + GetPerfCache.StartReportNow + "; PerfDeep.Start (" + GetPerfDeep.StartReportNow + "); LinuxTop.Stop (" + GetLinuxTop.Stop + ")" );
        }
        SetInternalCheckboxStatus ( "checkboxprofiling", GetProfiling);
        hideOrShow("row_profiling", fieldValue);
    } else if (fieldName == "checkboxmemory" ) {
        GetMemory.Value = fieldValue;
        SetInternalCheckboxStatus ( "checkboxmemory", GetMemory);
        // if the box is being unchecked, clear out the html so that it does not display when we check the box in the future
        if ( fieldValue == false ) {
            var objmemory = document.getElementById("MEMORY_HTML");
            if (objmemory) { objmemory.innerHTML = ""; }

            // when hiding, tell server to stop all data collection that might be going on
            GetPerfCache.StartReportNow = false;
            //AddToDebugOutput ( "checkboxprofiling: PerfCache.Start (" + GetPerfCache.StartReportNow + "; PerfDeep.Start (" + GetPerfDeep.StartReportNow + "); LinuxTop.Stop (" + GetLinuxTop.Stop + ")" );
        } else { // we are activating the Memory checkbox
            GetPerfCache.Value = 0;
            SetCheckboxStatus ( "checkboxPerfCache", GetPerfCache );
            SetCheckboxDisabled ( "checkboxPerfCache", false );
            GetSataUsb.Value = 0;
            SetCheckboxStatus ( "checkboxsatausb", GetSataUsb );
            SetCheckboxDisabled ( "checkboxsatausb", false );
            GetHeapStats.Value = 0;
            SetCheckboxStatus ( "checkboxheapstats", GetHeapStats );
            SetCheckboxDisabled ( "checkboxheapstats", false );
        }
        hideOrShow("row_memory", fieldValue);
    } else if (fieldName == "checkboxPerfTop" ) {
        GetPerfTop.Value = fieldValue;
        if (fieldValue) { // if turning on, turn others off
            GetLinuxTop.Value = 0;
            GetPerfCache.Value = 0;
            GetPerfDeep.Value = 0;
        }
        GetLinuxTop.Stop = 1;
        //alert("PerfTop (" + GetPerfTop.Value + "); LinuxTop (" + GetLinuxTop.Value + ")" );
        SetInternalCheckboxStatus ( "checkboxPerfTop", GetPerfTop);
        SetCheckboxDisabled ( "checkboxPerfDeep", fieldValue );
        SetCheckboxDisabled ( "checkboxPerfCache", fieldValue );
        SetCheckboxDisabled ( "checkboxLinuxTop", fieldValue );
    } else if (fieldName == "checkboxLinuxTop" ) {
        GetLinuxTop.Value = fieldValue;
        if (fieldValue) { // if turning on, turn others off
            GetPerfTop.Value = 0;
            GetPerfDeep.Value = 0;
        }
        //alert("PerfTop 1 (" + GetPerfTop.Value + "); LinuxTop (" + GetLinuxTop.Value + ")" );
        SetInternalCheckboxStatus ( "checkboxLinuxTop", GetLinuxTop);
        SetCheckboxDisabled ( "checkboxPerfDeep", fieldValue );
        SetCheckboxDisabled ( "checkboxPerfCache", fieldValue );
        SetCheckboxDisabled ( "checkboxPerfTop", fieldValue );

        if (fieldValue == false) { // if we are turning off LinuxTop, tell server to stop collecting data
            GetLinuxTop.Stop = 1;
        }
    } else if (fieldName == "checkboxPerfCache" ) {
        var objtable = document.getElementById("MEMORY_HTML");
        GetPerfCache.Value = fieldValue;
        checkboxPerfCacheDoit( fieldValue );
        if(debug) alert(fieldName + " checked; value " + fieldValue );
        GetPerfCache.StartReportNow = fieldValue;

        if (objtable) { objtable.innerHTML = "<textarea style=\"width:860px;\" ></textarea>"; } // turning on and off ... clear out the textarea

        if (fieldValue) { // if turning on
        } else { // else turning it off
        }
    } else if (fieldName == "checkboxPerfDeep" ) {
        GetPerfDeep.Value = fieldValue;
        checkboxPerfDeepDoit( fieldValue );
        if(debug) alert(fieldName + " checked; value " + fieldValue );
        GetPerfDeep.StartReportNow = fieldValue;
        if (fieldValue) { // if turning on, turn others off
            GetPerfCache.Value = 0;
            GetPerfTop.Value = 0;
            GetLinuxTop.Value = 0;
        }
        SetCheckboxDisabled ( "checkboxLinuxTop", fieldValue );
        SetCheckboxDisabled ( "checkboxPerfCache", fieldValue );
        SetCheckboxDisabled ( "checkboxPerfTop", fieldValue );
    } else if (fieldName == "checkboxContextSwitch" ) {
        SetInternalCheckboxStatus ( "checkboxContextSwitch", GetContextSwitch );
        hideOrShow("row_profiling_html", fieldValue);
    } else if (fieldName == "checkboxPerfFlame" ) {

        // when hiding flame graphs, tell server to stop all data collection that might be going on
        if (fieldValue == false) {
            if ( GetPerfFlame.State == GetPerfFlameState.IDLE ) { // if the flame graph is not recording presently
                GetPerfFlame.Value = false;
                GetPerfFlameSetStop();
                SetInternalCheckboxStatus ( "checkboxPerfFlame", GetPerfFlame);
                hideOrShow("row_PerfFlame", fieldValue);
                var objspan = document.getElementById('PerfFlameContents');
                if (objspan) {
                    objspan.innerHTML = ""; // clear out the contents element so old stuff does not display when we check the box in the future
                }
            } else {
                //alert("Please stop any recording before hiding the Flame Graph.");
            }
        } else {
            GetPerfFlame.Value = true;
            GetPerfFlameSetState( GetPerfFlameState.INIT ); // Init
            SetInternalCheckboxStatus ( "checkboxPerfFlame", GetPerfFlame);
            hideOrShow("row_PerfFlame", fieldValue);
        }
    } else if (fieldName == "checkboxWifiAmpduGraph" ) {
        checkboxWifiAmpduGraphDoit( fieldValue );
    } else if (fieldName == "checkboxiperfrow" ) {
        if (fieldValue == false) {
            iperfClientServerRow = 0;
            hideOrShow("row_iperf_client_server", HIDE );
        } else {
            iperfClientServerRow = 1;
            hideOrShow("row_iperf_client_server", SHOW );
        }
    } else if ( fieldName.indexOf ( "checkbox_netgfx" ) == 0 ) {
        var idx = fieldName.substr(15);
        if ( idx < 10 ) {
            NetGfxCheckbox[idx] = fieldValue;
            hideOrShow("row_netgfxsvg" + idx, fieldValue );
            hideOrShow("row_netgfxtxt" + idx, fieldValue );
            if (fieldValue) {
                hideOrShow("row_netgfx_all", fieldValue );
            } else {
                var found_row_visible = false;
                for ( var idx=0; idx<10; idx++) { // determine if at least one row is visible
                    if ( NetGfxCheckbox[idx] ) {
                        found_row_visible = true;
                        break;
                    }
                }
                if ( found_row_visible == false ) { // if not gfx rows are visible, hide the mother row
                    hideOrShow("row_netgfx_all", fieldValue );
                }
            }
        }
    } else if (fieldName == "checkboxDvfsControl" ) {
        hideOrShow("row_DvfsControl", fieldValue);

        if (fieldValue) {
            DvfsControl.Value = 1;
        } else {
            DvfsControl.Value = 0;
        }
    } else if (fieldName == "checkboxRecord" ) {
        hideOrShow("row_Record", fieldValue);
        // if the OneSecond timer is inactive and we are leaving Playback/Record, manually activate the OneSecond timer.
        if ( CgiTimeoutId == 0 && fieldValue == HIDE ) {
            OneSecond();
        }
    }

    if ( fieldValue ) {
        //alert("calling sendCgiRequest");
        sendCgiRequest();
    }
}

/**
 *  Function: This function will enable or disable the input buttons when a Playback
 *            is started. Disabling the inputs will prevent the user from altering the
 *            the web page and will allow the playback to proceed without any user
 *            changes.
 **/
function RecordPlaybackConfigure ( whichOne )
{
    var newState = false;
    clearTimeoutOneSecond( "RecordPlaybackConfigure");

    if ( whichOne == "START" ) {
        newState = true;
    } else {
        newState = false ;
    }

    setButtonDisabled( "checkboxDvfsControl", newState );
    setButtonDisabled( "checkboxmemory", newState );
    setButtonDisabled( "checkboxnets", newState );
    setButtonDisabled( "checkboxwifi", newState );
    setButtonDisabled( "checkboxirqs", newState );
    setButtonDisabled( "checkboxprofiling", newState );
    setButtonDisabled( 'checkboxPerfFlame', newState );

    return true;
}

/**
 *  Function: This function will change the button image file to the new specified
 *            image. It is used during playback to change the "play" button to the
 *            "pause" button and vice-a-versa.
 **/
function setButtonImage ( fieldName, newImageName )
{
    var obj = document.getElementById( fieldName );
    if ( obj ) {
        obj.src = newImageName;
    }
}

/**
 *  Function: This function will control the processing of all user-changeable
 *            elements ... buttons, images, checkboxes, input fields, etc.
 **/
function setVariable(fieldName)
{
    var debug=1;
    var fieldValue = "";
    //console.log ("setVariable: name " + fieldName );
    var obj=document.getElementById(fieldName);

    SetVariableCount++;

    if (debug) alert("setVariable: name " + fieldName + "; type " + obj.type + "value : " + obj.value);

    if ( PlaybackControl.Value ) { // if playback is in progress
        if ( (fieldName == "buttonRecordStart") || (fieldName == "buttonPlaybackRun") || (fieldName == "h1bsysperf") ||
             (fieldName == "hslogo") || (fieldName == "checkboxRecord" ) ) {
            // only allow these inputs to proceed during playback
        } else {
            return false;
        }
    }

    if (obj) {
        gFieldName = fieldName; // used to send the update to the CGI app

        if (obj.type == "checkbox" ) {
            fieldValue = obj.checked;
            checkboxSelected ( fieldName, fieldValue );
        } else if ( obj.type == "text" ) {
            fieldValue = obj.value;
        } else if ( obj.type == "textarea" ) {
            //alert("textarea ... " + fieldName );
            HideThisDisplayOther(fieldName);
        } else if ( obj.type == "radio" ) {
            var radios = document.getElementsByName( fieldName );

            fieldValue = document.querySelector('input[name=radioGovernor]:checked').value

            if ( fieldName.indexOf ( "radioGovernor" ) >= 0 ) {
                var objgovernor = document.getElementById( fieldName );
                if ( objgovernor ) {
                    GovernorSettingNow = fieldValue;
                    sendCgiRequest();
                }
            }
        } else if ( obj.type == "button" ) {
            //alert("button:" + fieldName);
            if (fieldName == "PerfFlameStartStop") {
                if (GetPerfFlame.State == GetPerfFlameState.IDLE) { // state is Idle
                    var objCmdLine = document.getElementById('PerfFlameCmdLine');
                    if (objCmdLine) {
                        if (objCmdLine.value.length > 0) {
                            GetPerfFlameSetState( GetPerfFlameState.START ); // Start (send over cmdline contents)
                            // change the text on the button from Start to Stop
                            var objButton = document.getElementById('PerfFlameStartStop');
                            if (objButton) {
                                objButton.value = "Stop";
                            }
                            var local = new Date();
                            GetPerfFlameRecordingSeconds = Math.floor(local.getTime() / 1000);

                            // disable the checkbox until the user stops the perf record
                            setButtonDisabled( 'checkboxPerfFlame', true );

                            // clear out the SVG container
                            objButton = document.getElementById('PerfFlameSvg');
                            if (objButton) {
                                objButton.innerHTML = "";
                            }
                        } else {
                            alert("The contents of the CmdLine box cannot be empty!");
                        }
                    }
                } else if (GetPerfFlame.State == GetPerfFlameState.RECORDING) { // state is Running/Recording
                    GetPerfFlameSetStop();
                }
            } else if (fieldName == "WifiScan") {
                if ( GetWifiScan.State == GetWifiScanState.UNINIT ) {
                    GetWifiScan.Value = true;
                    ClearOutHtml ( "WIFISCANRESULTS" );
                    GetWifiScan.State = GetWifiScanState.INIT;
                    setButtonDisabled ( fieldName, true );
                }
            } else if (fieldName == "iperf_start_stop_c") {
                var obj=document.getElementById(fieldName);
                if (obj) {
                    if (obj.value == "START") {
                        if ( document.getElementById('iperf_addr').value.length ) {
                            set_iperf_count_value( fieldName, "" );
                            iperfStateClient = iperfStateEnum.INIT;
                            if (iperfTimeoutClient) {
                                clearTimeout(iperfTimeoutClient);
                                iperfTimeoutClient = 0;
                            }
                            var obj_duration = document.getElementById('iperf_duration');
                            if (obj_duration && obj_duration.value ) {
                                var seconds = Number(obj_duration.value * 1000);
                                iperfTimeoutClient = setTimeout ( "set_iperf_stop()", seconds );
                            }
                            KeyupEntryBox( fieldName );
                        } else {
                            alert("You must specify a Server Address before starting iperf.");
                        }
                    } else {
                        iperfStateClient = iperfStateEnum.STOP;
                    }
                    iperfPidClient = "";
                    set_iperf_button( fieldName );
                    sendCgiRequest();
                }
            } else if (fieldName == "iperf_start_stop_s") {
                var obj=document.getElementById(fieldName);
                if (obj) {
                    if (obj.value == "START") {
                        set_iperf_count_value( fieldName, "" );
                        iperfStateServer = iperfStateEnum.INIT;
                        KeyupEntryBox( fieldName );
                    } else {
                        iperfStateServer = iperfStateEnum.STOP;
                    }
                    iperfPidServer = "";
                    set_iperf_button( fieldName );
                    sendCgiRequest();
                }
            } else if (fieldName == "buttonRecordStart") {
                var obj=document.getElementById(fieldName);
                if (obj) {
                    if (obj.value.indexOf( "Start") == 0 ) {
                        var localtime = new Date();
                        RecordTimeStart = Math.floor(localtime.getTime() / 1000);
                        RecordControl.Value = 1; // init recording
                        obj.value = "Stop Record";
                        obj.style.backgroundColor = "salmon";
                        setButtonDisabled( 'RecordSaveFile', true );

                        setButtonDisabled( 'buttonExportCpuStats', true );
                        setButtonDisabled( 'buttonExportNetStats', true );
                        setButtonDisabled( 'buttonChooseFile', true );

                        RecordPlaybackConfigure ( "START" );

                        // clear the contents in case we had a previous recording
                        RecordFileContents = "";

                        PlaybackTimeSeconds = Math.floor(localtime.getTime() / 1000);
                        RecordFileContents += "\nUNIXTIME " + PlaybackTimeSeconds + "|!|";
                        RecordFileContents += "PLATFORM~" + PlaybackPLATFORM;
                        RecordFileContents +=  "~VARIANT~" + PlaybackVARIANT;
                        RecordFileContents +=  "~PLATVER~" + PlaybackPLATVER;
                        RecordFileContents +=  "~VERSION~" + PlaybackVERSION;
                        RecordFileContents +=  "~UNAME~" + PlaybackUNAME;
                        RecordFileContents +=  "~BOLTVER~" + PlaybackBOLTVER;
                        //RecordFileContents +=  "~CPUPERCENTS~" + PlaybackCPUPERCENTS;
                        RecordFileContents +=  "~netStatsInit~" + PlaybacknetStatsInit;
                        RecordFileContents +=  "~STBTIME~" + PlaybackSTBTIME;
                        RecordFileContents += "\n";
                        PlaybackSTBTIMEsaved = PlaybackSTBTIME;
                        RecordFileSize = RecordFileContents.length;

                        sendCgiRequest();
                    } else {
                        RecordControl.Value = 0; // stop recording
                        obj.value = "Start Record";
                        obj.style.backgroundColor = "lightgreen";
                        setButtonDisabled( 'RecordSaveFile', false );

                        RecordPlaybackConfigure ( "STOP" );

                        setButtonDisabled( 'buttonExportCpuStats', false );
                        setButtonDisabled( 'buttonExportNetStats', false );
                        setButtonDisabled( 'buttonChooseFile', false );

                        RecordFileContentsArray = RecordFileContents.split("UNIXTIME ");
                    }
                }
            } else if (fieldName == "buttonExportCpuStats") {

                // process is handled in function ExportCpuStatsListener()

            } else if (fieldName == "buttonExportNetStats") {

                // process is handled in function ExportNetStatsListener()

            }
        } else if ( obj.type == "image" ) {
            //console.log( fieldName + ": current state:" + PlaybackControl.Value );
            if (fieldName == "buttonPlaybackRun") {
                if ( PlaybackControl.Value == 0 ) {
                    //alert("Start Playback");

                    PlaybackControl.Value = 1;
                    //console.log( fieldName + "; new state:" + PlaybackControl.Value );

                    clearOutCpuGraphsAndTextareas();

                    RecordPlaybackConfigure ( "START" );

                    PlaybackFileContentsArrayIdx = 1; // the first entry is always zero length

                    FindInputElementsThatAreEnabled ();

                    ProcessPlaybackEvents();

                    PlaybackElementsSetDisabled ( true );

                    setButtonDisabled( 'checkboxRecord', true ); // make sure the Record checkbox is disabled

                } else if ( PlaybackControl.Value == 2 ) { // we are paused and need to resume playing

                    PlaybackControl.Value = 1;
                    //console.log( fieldName + "; new state:" + PlaybackControl.Value );
                    ProcessPlaybackEvents();

                    setButtonDisabled( 'checkboxRecord', true ); // make sure the Record checkbox is disabled

                } else { // we are running and someone hit pause
                    if ( PlaybackTimeoutId ) { clearTimeout( PlaybackTimeoutId ); }

                    PlaybackControl.Value = 2;
                    //console.log( fieldName + "; new state:" + PlaybackControl.Value );

                    setButtonDisabled( 'checkboxRecord', false ); // make sure the Record checkbox is enabled
                }
            }
        } else if ( obj.type == "select-one" ) {
        } else if (fieldName.indexOf("ChangeCpuState") == 0 ) { // user clicked disable/enable CPU
            ChangeCpuState = fieldName;
            //console.log( fieldName + ": gNumCpus:" + gNumCpus );

            if ( PlaybackControl.Value == 0 ) { // if we are NOT doing a playback
                var iconobj = document.getElementById(fieldName);
                var iconfill = "unknown";
                if (iconobj) {
                    var temp;

                    iconfill = iconobj.getAttribute('fill');
                    // Based on the current color of the circle that was clicked, we will either be enabling or disabling the specified CPU.
                    // A green (lime) circle means we want the CPU to go active; a red circle means we want the CPU to stop.
                    if ( iconfill.indexOf("lime") == -1 ) {
                        temp = ChangeCpuState.replace(/ChangeCpuState/, "ChangeCpuState=-");
                    } else {
                        temp = ChangeCpuState.replace(/ChangeCpuState/, "ChangeCpuState=+");
                    }
                    ChangeCpuState = temp;
                }
                //alert("Checked: (" + fieldName + "); src (" + iconfill + "); ChangeCpuState (" + ChangeCpuState + "); indexOf(lime)=" + iconfill.indexOf("lime") );
                sendCgiRequest();
            }
        }

        if (fieldName == "h1bsysperf") {
            MasterDebug = 1-MasterDebug;
            //alert("MasterDebug " + MasterDebug + "; objdebug " + objdebug );
            if (MasterDebug==1) {
                if (objdebug) {
                    objdebug.style.visibility = "";
                    GlobalDebug = 0;
                }
            } else {
                if (objdebug) {
                    objdebug.style.visibility = "hidden";
                    GlobalDebug = 0;
                }
                var CgiCountObj = document.getElementById('cgicount');
                if (CgiCountObj) {
                    CgiCountObj.innerHTML = "";
                }
            }
        } else if ( fieldName == "hslogo" ) {
            window.location.href = "splice.html";
        }
    }
    //alert("setVariable: done");
}

function randomIntFromInterval(min,max)
{
    return Math.floor(Math.random()*(max-min+1)+min);
}

function sendCgiRequestDoItNow( url )
{
    var debug=0;

    if (debug==1) console.log ("sending len (" + url.length + ") (" + url + ")" );

    xmlhttp=new XMLHttpRequest();
    xmlhttp.onreadystatechange= serverHttpResponse;
    xmlhttp.open("GET",url,true);
    xmlhttp.send(null);

    CgiCount++;
    var CgiCountObj = document.getElementById('cgicount');
    if ( MasterDebug && CgiCountObj) {
        CgiCountObj.innerHTML = "&nbsp;&nbsp;(" + CgiCount + "," + SetVariableCount + "," + ResponseCount + ")";
    }
}

function sendCgiRequest( )
{
    var debug=0;

    var idx=1;
    var url = "";

    var RandomValue = randomIntFromInterval(1000000,9999999);
    url = "/cgi/splice.cgi?randomvalue=" + RandomValue;

    if (epochSeconds > 0) {
        url += "&datetime=" + epochSeconds + "&tzoffset=" + tzOffset;
        epochSeconds = 0;
    }

    url += "&uuid=" + UUID;

    if (GetCpuInfo.Value == true) {
        url += "&cpuinfo=1";
    }

    if (GetIrqInfo.Value == true) {
        url += "&irqinfo=1";
    }

    var checkboxiperf=document.getElementById('checkboxiperfrow');
    if (GetNetStats.Value == true) {
        if ( GetNetStatsInit ) {
            url += "&netStatsInit=1";
            GetNetStatsInit = false;
        }
        url += "&netStatsUpdate=1";
        if (iperfInit) {
            url += "&iperfInit=1";
            iperfInit = false;
        }
        if ( checkboxiperf && checkboxiperf.checked == true ) {
            url += "&iperfRunningClient=1&iperfRunningServer=1";
        }
    }

    if ( iperfStateClient == iperfStateEnum.INIT ) {
        url += "&iperfCmdLine=" + CreateIperfString( "Client" ) + " > /tmp/iperf_client_" + UUID + ".log";
        iperfStateClient = iperfStateEnum.RUNNING;
    } else if ( iperfStateClient == iperfStateEnum.STOP ) {
        url += "&iperfCmdLine=STOP iperf -c";
        iperfStateClient = iperfStateEnum.UNINIT;
        if ( iperfPidClient != "terminated" && iperfPidClient != "Executable not found" ) iperfPidClient = "";
        set_iperf_status_cell();
        set_iperf_button( "Client" );
    } else if ( iperfStateClient == iperfStateEnum.RUNNING && GetNetStats.Value && checkboxiperf && checkboxiperf.checked == true ) {
        url += "&iperfPidClient=1";
    }

    if ( iperfStateServer == iperfStateEnum.INIT ) {
        url += "&iperfCmdLine=" + CreateIperfString( "Server" );
        iperfStateServer = iperfStateEnum.RUNNING;
    } else if ( iperfStateServer == iperfStateEnum.STOP ) {
        url += "&iperfCmdLine=STOP iperf -s";
        iperfStateServer = iperfStateEnum.UNINIT;
        iperfStateServerRetry = 0;
        if ( iperfPidServer != "terminated" && iperfPidServer != "Executable not found" ) iperfPidServer = "";
        set_iperf_status_cell();
        set_iperf_button( "Server" );
    } else if ( iperfStateServer == iperfStateEnum.RUNNING && GetNetStats.Value && checkboxiperf && checkboxiperf.checked == true ) {
        url += "&iperfPidServer=1";
    }

    var obj2 = document.getElementById("checkboxwifi");
    if (obj2  && obj2.checked ) {
        if (GetWifiStats.Init == true) {
            url += "&wifiinit=1"; // the first time after the box is checked, request chip/driver version, board id, etc. ... stuff that does not change
            GetWifiStats.Init = false;
        }
        if (GetWifiStats.Value == true) {
            url += "&wifiStats=1";

            if ( GetWifiAmpduGraph.Value == 1 ) {
                url += "&wifiAmpduGraph=1";
            }
        }

        if ( GetWifiAmpduGraph.FirstTime == 1) {
            url += "&wifiAmpduStart=1";
            GetWifiAmpduGraph.FirstTime = 2; // after we have told bsysperf_server to begin collecting data, do not do it again. wifiStats will be used thereafter
        }

        if (GetWifiScan.Value == true) {
            url += "&wifiscanstart=1";
            GetWifiScan.Value = false;
            GetWifiScan.State = GetWifiScanState.SCANNING;
            GetWifiScan.ServerIdx = 0;
        }

        if (GetWifiScan.State == GetWifiScanState.SCANNING ) {
            url += "&wifiscanresults=" + GetWifiScan.ServerIdx;
        }
    }

    if (GetSysInfo.Value == true) {
        url += "&systemInfo=1";
    }

    if (GetTime.Value == true) {
        url += "&getTime=1";
    }

    if (GetHeapStats.Value == true) {
        url += "&heapstats=1";
    }

    if (GetSataUsb.Value == true) {
        url += "&satausb=1";
    }

    if (GetSataUsb.Stop == 1) {
        url += "&satausb=2";
        GetSataUsb.Stop = 0;
    }

    if (GetProfiling.Value == true) {
        url += "&profiling=1";
        GetProfiling.Value = false;
    }

    if (GetMemory.Value == true) {
        url += "&memory=1";
        GetMemory.Value = false;
    }

    if (GetLinuxTop.Stop == true && RecordControl.Value == 0 /* we are not recording */) {
        url += "&LinuxTop=2";
        AddToDebugOutput ( "sendCgiRequest: LinuxTop.Stop (" + GetLinuxTop.Stop + ")" );
        GetLinuxTop.Stop = 0;
    } else if (GetLinuxTop.Value == true) {
            url += "&LinuxTop=1";
    }

    if (GetContextSwitch.Value == true ) {
            url += "&ContextSwitch=1";
    }

    // if profiling is displayed and kernel has been compiled with perf enabled
    if (GetCheckboxStatus ( "checkboxprofiling" ) && PerfError == false) {

        //alert("sendCgi: GetPerfDeep.Value (" + GetPerfDeep.Value + ")" );
        if (GetPerfDeep.Value == true && GetPerfDeep.Duration > 0 && GetPerfDeep.StartReportNow == true ) {
            url += "&PerfDeep=" + GetPerfDeep.Duration;
            GetPerfDeep.StartReportNow = false;
        }

        //alert("sendCgi: GetPerfDeepResults.Value (" + GetPerfDeepResults.Value + ")" );
        if (GetPerfDeepResults.Value == true ) {
            url += "&PerfDeepResults=1";
            GetPerfDeepResults.Value = false;
        }

        if (GetPerfTop.Value == true) {
            url += "&PerfTop=1";
            //GetPerfTop.Value = 0; // changed so that the user must un-check the box to get PerfTop to stop updating
        }
    }

    if(debug) alert("checkboxmemory (" + GetCheckboxStatus ( "checkboxmemory" ) + "); PerfError (" + PerfError + ")" );
    // if memory row is displayed and kernel has been compiled with perf enabled
    if (GetCheckboxStatus ( "checkboxmemory" ) && (PerfError == false) ) {

        if(debug) alert("sendCgi: GetPerfCache.Value (" + GetPerfCache.Value + ")" );
        if (GetPerfCache.Value == true && GetPerfCache.Duration > 0 && GetPerfCache.StartReportNow == true ) {
            url += "&PerfCache=" + GetPerfCache.Duration;
            GetPerfCache.StartReportNow = false;
        }

        if(debug) alert("sendCgi: GetPerfCacheResults.Value (" + GetPerfCacheResults.Value + ")" );
        if (GetPerfCacheResults.Value == true ) {
            if(debug) alert("sendCgi: GetPerfCache.Value (" + GetPerfCache.Value + ")" );
            url += "&PerfCacheResults=1";
            GetPerfCacheResults.Value = false;
        }
    }

    // if flame is displayed and kernel has been compiled with perf enabled
    if (GetCheckboxStatus ( "checkboxPerfFlame" ) && PerfError == false ) {

        // states are: Init:1, Idle:2, Start:3, Recording:4, Stop:5
        if (GetPerfFlame.State != GetPerfFlameState.IDLE ) { // if NOT idle
            url += "&PerfFlame=" + GetPerfFlame.State;

            if (GetPerfFlame.State == GetPerfFlameState.START) {
                var objCmdLine = document.getElementById('PerfFlameCmdLine');
                if (objCmdLine) {
                    url += "&PerfFlameCmdLine=" + encodeURIComponent(objCmdLine.value);
                    GetPerfFlameSetState( GetPerfFlameState.RECORDING );
                }
                GetPerfFlameSvgCount++;
            }

            if (GetPerfFlame.State == GetPerfFlameState.DELETEOUTFILE) {
                url += "&PerfFlameSvgCount=" + GetPerfFlameSvgCount;
                url += "&perf_out=" + PerfRecordUuid;
            }

            if (GetPerfFlame.State == GetPerfFlameState.CREATESCRIPTOUT) {
                url += "&perf_out=" + PerfRecordUuid;
            }
        }

        if (GetPerfFlame.State == GetPerfFlameState.INIT ) {
            GetPerfFlameSetState( GetPerfFlameState.IDLE );
        }
    } else if (GetPerfFlame.State == GetPerfFlameState.STOP) { // if we hid the flame graph window while a record was in progress
        url += "&PerfFlame=" + GetPerfFlame.State;
        GetPerfFlameSetState( GetPerfFlameState.IDLE );
    }

    if (ChangeCpuState.length > 0) {
        url += "&" + ChangeCpuState;
        //alert("sendCgiRequest: ChangeCpuState (" + ChangeCpuState + ")" );
        ChangeCpuState = "";
    }

    if ( DvfsControl.Value ) {
        url += "&DvfsControl=" + DvfsControl.Value;
    }

    if ( GovernorSettingNow ) {
        url += "&GovernorSetting=" + GovernorSettingNow;
        GovernorSettingPrev = GovernorSettingNow;
        GovernorSettingNow = 0;
    }

    // seems the data sent to the browser cannot exceed 1010 for unknown reason
    if (url.length > 1010) {
        url = url.substr(0,1010);
    }

    urlSentPreviously = url;

    sendCgiRequestDoItNow ( url );
}

function MsIeCrFix ( newstring )
{
    //if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, must replace carriage returns with <br>
    //    return newstring.replace(/\n/g, "<br>");
    //}
    return newstring;
}

function AddToDebugOutput ( newstring )
{
    var debug = 1;

    //alert("AddToDebug: objdebug " + objdebug + "); newstring (" + newstring + ")" );
    if (debug && objdebug) {
        objdebug.innerHTML += MsIeCrFix ( newstring );
    }
}

function RetryLinuxTop()
{
    //alert("RetryLinuxTop: GetLinuxTop.Value (" + GetLinuxTop.Value + ")" )
    if (GetLinuxTop.Value) {
        sendCgiRequest();
    }
}

function GetPerfDeepResultsDoit()
{
    //alert("GetPerfDeepResultsDoit: countdown " + GetPerfDeepCountdown );
    if ( GetPerfDeepCountdown > 1) {
        GetPerfDeepCountdown--;
        //alert( "GetPerfDeepResultsDoit: duration " + GetPerfDeepCountdown );
        setTimeout ('GetPerfDeepResultsDoit()', 1000 );
    } else {
        GetPerfDeepResults.Value = true;
        sendCgiRequest();
    }
}

function GetPerfCacheResultsFunc()
{
    //alert("GetPerfCacheResultsFunc: countdown " + GetPerfCacheCountdown );
    if ( GetPerfCacheCountdown > 1) {
        GetPerfCacheCountdown--;
        //alert( "GetPerfCacheResultsFunc: duration " + GetPerfCacheCountdown );
        setTimeout ('GetPerfCacheResultsFunc()', 1000 );
    } else {
        GetPerfCacheResults.Value = true;
        sendCgiRequest();
    }
}

function ComputePrecision ( Mbps )
{
    var precision = 2;
    var integerPart = Mbps.toFixed(0);
    var decimalPoint = ""; // if the integer part is 4 digits or more, add the decimal point after the integer

    // if the Mbps is less than one Mbps, increase the decimal precision to show kilobits
    if ( integerPart == 0 ) {
        precision = 3;
    } else if ( integerPart.length > 3 ) {
        precision = 0;
        decimalPoint = ".";
    } else if ( integerPart.length > 2 ) {
        precision = 1;
    }

    return [ precision, decimalPoint ];
}

function ConvertBitsToMbps ( bits )
{
    var Mbps      = 0;
    var precision = 2;
    var decimalPoint = ""; // if the integer part is 4 digits or more, add the decimal point after the integer

    Mbps =  Number( bits / 1024 / 1024 );

    var return_values = ComputePrecision ( Mbps );
    precision = return_values[0];
    decimalPoint = return_values[1];

    return Mbps.toFixed(precision) + decimalPoint;
}

function ComputeNetBytes10SecondsAverage ( InterfaceIndex, RxTxIndex )
{
    var averageMbps     = 0;
    var secs            = 0;
    var cummulativeMbps = 0;
    var precision       = 0;
    var decimalPoint = ""; // if the integer part is 4 digits or more, add the decimal point after the integer

    //if (objdebug && InterfaceIndex == 1 ) objdebug.innerHTML += "ComputeAverage: num secs " + NetBytesRx10SecondsCount[InterfaceIndex] + ": ";
    for(secs=0; secs<NetBytesRx10SecondsCount[InterfaceIndex]; secs++) {
        if (RxTxIndex == 0) {
            cummulativeMbps += NetBytesRx10Seconds[InterfaceIndex][secs];
        } else {
            cummulativeMbps += NetBytesTx10Seconds[InterfaceIndex][secs];
        }
    }
    if ( NetBytesRx10SecondsCount[InterfaceIndex] > 0) {
        averageMbps = Number ( cummulativeMbps / NetBytesRx10SecondsCount[InterfaceIndex] );
    }

    var return_values = ComputePrecision ( averageMbps );
    precision = return_values[0];
    decimalPoint = return_values[1];

    return averageMbps.toFixed(precision) + decimalPoint;
}

// This function will insert SVG code into the appropriate HTML structure to either draw a red or green
// circle. The circle will be clicked to enable or disable the CPU.
function ChangeCpuTag ( idx, idle )
{
    var CpuObj = document.getElementById('ChangeCpuTag' + idx );
    if (CpuObj && idx > 0) {
        if ( idle == 255 ) {
            CpuObj.innerHTML = "<svg height=20 width=20 ><circle cx=10 cy=10 r=10 onclick=\"MyClick(event);\" id=ChangeCpuState" + idx + " fill=\"lime\" /></svg>";
        } else {
            CpuObj.innerHTML = "<svg height=20 width=20 ><circle cx=10 cy=10 r=10 onclick=\"MyClick(event);\" id=ChangeCpuState" + idx + " fill=\"red\" /></svg>";
        }
        //alert("for CPU " + idx + ", html is (" + CpuObj.innerHTML + ")" );
    }
}

function sendCgiRequestRetry ( )
{
    if ( CgiRetryTimeoutId ) {
        //console.log( "sendCgiRequestRetry: clearTimeout(CgiRetryTimeoutId:" + CgiRetryTimeoutId + ")");
        clearTimeout ( CgiRetryTimeoutId );
        CgiRetryTimeoutId = 0;
    }

    //console.log ( "sendCgiRequestRetry: calling setTimeout(sendCgiRequestRetry(2secs)); ID (" + CgiRetryTimeoutId + ")" + eol );
    CgiRetryTimeoutId = setTimeout ('sendCgiRequestRetry()', REFRESH_IN_MILLISECONDS*2 );

    // send the previously attempted url request
    //console.log ( "sendCgiRequestRetry: Retry url: " + urlSentPreviously + eol );
    sendCgiRequestDoItNow( urlSentPreviously );
}

function ComputeTotalCpuLongAverage()
{
    var idx=0;
    var idxMax=cpuUsageLongAverageCount;
    var totalUtilization = 0;
    var averageUtilization = 0;


    if (cpuUsageLongAverageCount) {
        if ( cpuUsageLongAverageCount >= CPU_USAGE_LONG_AVERAGE_MAX ) idxMax = CPU_USAGE_LONG_AVERAGE_MAX;
        for (idx=0; idx<idxMax; idx++) {
            totalUtilization += Number ( cpuUsageLongAverage[idx] );
        }
        averageUtilization = Number ( totalUtilization / idxMax ).toFixed(1);
    }

    return averageUtilization;
}

// This function runs after the SVG iframe has been loaded with the contents from the Perl script.
// It gives us a chance to do any follow-up work once the SVG had rendered.
function iframeLoad ()
{
    GetPerfFlameSetState( GetPerfFlameState.DELETEOUTFILE );

    //alert("iframeLoad");
    //var iframe = document.getElementById('iframe123');
}

function GetSvgContents ()
{
    var ipaddr = window.location.hostname;
    var perl_server = 'http://home.irv.broadcom.com/~detrick';
    var fg_script = 'fg_generator.cgi'
    var perf_out = PerfRecordUuid + '.out';
    var url = perl_server + '/' +fg_script+ '?ipaddr=' + ipaddr + '&perf_out=' + perf_out;
    var targetDiv = document.getElementById('PerfFlameSvg'); // div002

    if ( targetDiv ) {
        targetDiv.innerHTML = "";
        var newFrame = document.createElement("iframe");
        newFrame.setAttribute("src", url );
        newFrame.setAttribute("id", "iframe123" );
        newFrame.setAttribute("onload", "iframeLoad()" );
        newFrame.style.width="100%";
        newFrame.style.height="800px";
        newFrame.style.border="thick solid #ffffff"; // green #00cc00
        targetDiv.appendChild(newFrame);
    }
}

function HideThisDisplayOther( id )
{
    var len = id.length;
    if ( len > 1 ) {
        var sub=id.substr(len-1,1);
        var other = "";
        if ( sub == "t" ) { // if this is the title
            // abc123t
            other = id.substr(0,len-1) + "d"; // other is the detailed one
        } else {
            other = id.substr(0,len-1) + "t"; // other is the title one
        }
        //alert("id is (" + id + ") ... len (" + len + ") ... sub (" + sub + ") other (" + other + ")" );
        var obj_this_row = document.getElementById(id + "r");
        var obj_other_row = document.getElementById(other + "r");
        //alert("id_row (" + id + "r" + ") - obj_this_row:" + obj_this_row + "\nid_row (" + other + "r" + ") ... obj_other_row:" + obj_other_row);
        if ( obj_this_row && obj_other_row ) {
            obj_this_row.style.display = "none";
            obj_other_row.style.display = "";
        } else {
            //alert("obj_this_row:" + obj_this_row + " ... obj_other_row:" + obj_other_row);
        }
    }
}

var swap_pass=0;
function SwapElementsValues( id1, id2 )
{
    var obj1 = document.getElementById( id1 );
    var obj2 = document.getElementById( id2 );
    //if ( swap_pass == 0 ) alert("SwapElementsValues: id1:" + id1 + ";   id2:" + id2 + ";  obj1:" + obj1 + ";  obj2:" + obj2 );
    var temp = 0;

    if (obj1 && obj2) {
        //if ( swap_pass == 0 ) alert("SwapElementsValues: before obj1.value:" + obj1.value + ";  obj2.value:" + obj2.value );
        temp = obj1.value;
        obj1.value = obj2.value;
        obj2.value = temp;
        //if ( swap_pass == 0 ) alert("SwapElementsValues: after obj1.value:" + obj1.value + ";  obj2.value:" + obj2.value );
    }
}
function SwapElementsInnerHTML( id1, id2 )
{
    var obj1 = document.getElementById( id1 );
    var obj2 = document.getElementById( id2 );
    var temp = 0;

    if (obj1 && obj2) {
        //if ( swap_pass == 0 ) alert("SwapElementsValues: before obj1.innerHTML:" + obj1.innerHTML+ ";  obj2.innerHTML:" + obj2.innerHTML );
        temp = obj1.innerHTML;
        obj1.innerHTML = obj2.innerHTML;
        obj2.innerHTML = temp;
        //alert("for id1 (" + id1 + ") ... id1.indexOf(s_x)=" + id1.indexOf("s_x") );
        if ( id1.indexOf("s_x") == 0 || id1.indexOf("S_x") == 0 ) {
            var rowid1 = "r" + id1.substr(1,99);
            var rowid2 = "R" + id2.substr(1,99);
            var objrow1 = document.getElementById( rowid1 );
            var objrow2 = document.getElementById( rowid2 );
            //alert("for rowid1 (" + rowid1 + ")   rowid2 (" + rowid2 + ") ... objrow1=" + objrow1 + ";   objrow2=" + objrow2 );
            if(objrow1 && objrow2) {
                temp = objrow1.getAttribute('style');
                objrow1.setAttribute('style', objrow2.getAttribute('style'));
                objrow2.setAttribute('style', temp);
                //alert("SwapInnerHTML: (" + temp +"); row.style=(" + objrow1.getAttribute('style') + ");  row2:(" + objrow2.getAttribute('style') + ")  rowid1=(" + rowid1 + ")" );
            }
        }
        //if ( swap_pass == 0 ) alert("SwapElementsValues: after obj1.innerHTML:" + obj1.innerHTML+ ";  obj2.innerHTML:" + obj2.innerHTML );
    }
}

function SwapTxRx( which )
{
    var ii, jj;
    var id1 = "";
    var id2 = "";

    //alert("SwapTxTx: which=" + which );
    for(ii=0; ii<8; ii++) {
        if (which == 0) {
            id1 = "s_x" + ii;
            id2 = "S_x" + ii;
        } else {
            id1 = "S_x" + ii;
            id2 = "s_x" + ii;
        }
        SwapElementsInnerHTML( id1, id2 );

        if (which == 0) {
            id1 = "c_x" + ii;
            id2 = "C_x" + ii;
        } else {
            id1 = "C_x" + ii;
            id2 = "c_x" + ii;
        }
        SwapElementsValues( id1, id2 );

        for(jj=0; jj<12; jj++) {
            if (which == 0) {
                id1 = "v" + ii + "_" + jj;
                id2 = "V" + ii + "_" + jj;
            } else {
                id1 = "V" + ii + "_" + jj;
                id2 = "v" + ii + "_" + jj;
            }
            SwapElementsValues( id1, id2 );
        }
        swap_pass++;
    }
    //alert("SwapTxRx done");
}

function UpdateDashTickText ( svg_text_id, max_value )
{
    // tags should be something like this ... dash_rx_0 .. dash_rx_9 ... or same with tx
    var text_value = "";
    var multiplier = [10,30,50,70,90];
    var full_id = svg_text_id + "_";
    var objtext = 0;
    var yidx = 0; // the y coordinates are the reverse of idx values

    // loop through 5 text values and set the text values to 90%, 70%, 50%, 30%, 10% of the new max_value
    for ( var idx=0; idx<5; idx++) {
        text_value = Number ( max_value * multiplier[idx] / 100 );
        yidx = Number(4-idx);
        full_id = svg_text_id + "_" + yidx;
        objtext = document.getElementById( full_id );
        if (objtext) {
            var shorter_text = text_value.toString().replace(/000000000$/, "G");
            shorter_text = shorter_text.toString().replace(/000000$/, "M");
            shorter_text = shorter_text.toString().replace(/000$/, "K");
            objtext.textContent = shorter_text;
        }
    }

    return true;
}

function ComputeNewNetGfxDivisor ( RxTxIndex, netIfIdx, newValue )
{
    var prev_divisor = NetGfxDivisor[RxTxIndex][netIfIdx];
    var temp = Number ( newValue / 100 );
    var newValueStr = newValue.toString();
    var log10 = Math.max ( 1, Number ( newValueStr.length - 2 ) ); // Math.ceil ( Math.log10 ( temp ));
    //var log10_chrome_ff_safari = Math.ceil ( Math.log10 ( temp )); // fails on IE9 and Safari Windows
    NetGfxDivisor[RxTxIndex][netIfIdx] = temp = Number ( Math.pow ( 10, log10 ) );

    return [ prev_divisor, NetGfxDivisor[RxTxIndex][netIfIdx] ];
}

function ResetPolylinePoints ( objline, netIfIdx, RxTxIndex )
{
    var delta2 = 0;
    var coords = "";
    var newcoords = "";

    if ( objline ) {
        coords = objline.getAttribute('points' );
        var bytes = NetGfxBytes[RxTxIndex][netIfIdx].split(" ");
        var max_idx = Math.min(POINTS_PER_GRAPH, bytes.length);

        ComputeNewNetGfxDivisor ( RxTxIndex, netIfIdx, bytes[0] );

        for (var idx=0; idx < max_idx; idx++ ) {
            delta2 = Math.floor ( Number ( bytes[idx] / NetGfxDivisor[RxTxIndex][netIfIdx] ) );
            if (delta2 && delta2<=100) {
                delta2 = Number(100 - delta2);
            } else {
                delta2 = 100;
            }
            newcoords += Number(idx+1)*2 + "," + delta2 + " " ;
        }
        objline.setAttribute('points', newcoords );
    }

    return true;
}

function AppendNetGfxBytes(RxTxIndex, netIfIdx, delta1 )
{
    // if we have already saved some values in the y-axis array
    if ( NetGfxBytes[RxTxIndex][netIfIdx].length ) {
        // determine how many entries are in the string (separated by spaces)
        var entries = NetGfxBytes[RxTxIndex][netIfIdx].split(" ");
        var num_entries = entries.length;
        // if we have not filled up the width of the graph histogram, add this entry to the end ... delimiting with a space
        if ( num_entries < POINTS_PER_GRAPH ) {
            NetGfxBytes[RxTxIndex][netIfIdx] += " " + delta1;
        } else { // drop the first entry in the list and add new entry at the end
            var first_space = NetGfxBytes[RxTxIndex][netIfIdx].indexOf(" ");
            if ( first_space > 0 ) {
                NetGfxBytes[RxTxIndex][netIfIdx] = NetGfxBytes[RxTxIndex][netIfIdx].substr( Number(first_space + 1) );
            } else {
                NetGfxBytes[RxTxIndex][netIfIdx] += " " + delta1;
            }
        }
    } else {
        NetGfxBytes[RxTxIndex][netIfIdx] += delta1;
    }
}

function AdjustPolylinePoints ( objline, old_divisor, new_divisor )
{
    if (objline && (new_divisor > old_divisor) ) {
        var delta_divisor = Number ( new_divisor / old_divisor ); // going from 100 to 1000 means we have to divide all the existing y values by 10
        var coords = objline.getAttribute('points');
        if (coords) {
            var splits = coords.split(' ');
            var newY = "";
            var new_points = "";

            for (var idx=0; idx < splits.length; idx++ ) {
                var justone = splits[idx].split(',');
                if ( justone.length == 2 ) {
                    if ( idx > 0 ) {
                        new_points += " ";
                    }
                    newY = Number ( 100 - Math.floor ( Number( ( 100 - justone[1] ) / delta_divisor ) ) );
                    new_points += idx*2 + "," + newY;
                }
            }
            objline.setAttribute('points', new_points );
        }
    }
    return true;
}

/*
 * This function will append the specified value to the end of the text area element. When the text area element contains
 * 250 values, the first value will be dropped off. Then, the new value will be appended to the end ... thereby ensuring
 * that a maximum of 250 values will be the in text area.
*/
function AppendToTextarea ( textareaId, newValue )
{
    var objtxt = document.getElementById( textareaId );

    if (objtxt) {
        var values = objtxt.innerHTML.split(' ');

        if ( values.length >= 250 ) {
            var firstDelimiter = objtxt.innerHTML.indexOf(' ');
            if (firstDelimiter > 0 ) {
                objtxt.innerHTML = objtxt.innerHTML.substr ( Number ( firstDelimiter + 1 ) );
            }
        }

        if ( objtxt.innerHTML.length > 1 ) {
            objtxt.innerHTML += ' ';
        }
        //objtxt.innerHTML += delta1.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",") + " "; // format number with commas
        objtxt.innerHTML += newValue;
        objtxt.scrollTop = objtxt.scrollHeight;
    }

    return true;
}

function UpdateNetGfxElements ( netIfIdx, RxTxIndex, previousBytes, currentBytes )
{
    var temp = 0;

    if ( NetGfxCheckbox[netIfIdx] && 0 <= RxTxIndex && RxTxIndex <=1 && currentBytes >= previousBytes ) {
        var objbox = document.getElementById( 'checkbox_netgfx' + netIfIdx );
        if ( objbox ) {
            var delta1 = Math.floor ( Number ( (currentBytes - previousBytes) * 8 ) );
            var objline = document.getElementById( 'polyline_netgfx_' + RxTxStr[RxTxIndex] + '_' + netIfIdx );

            if ( delta1 < 0 ) delta1 = 0; // do not allow the num bytes to go negative

            objbox.checked = true;

            // determine if the divisor has to change
            if ( Number (delta1 / NetGfxDivisor[RxTxIndex][netIfIdx] ) > 100 ) {
                var returns = ComputeNewNetGfxDivisor ( RxTxIndex, netIfIdx, delta1 ); // returns old and new values
                AdjustPolylinePoints ( objline, returns[0], returns[1] );
                UpdateDashTickText ( "dash_" + RxTxStr[RxTxIndex] + "_" + netIfIdx, Number ( NetGfxDivisor[RxTxIndex][netIfIdx] * 100 ) );
            }

            // value could be quite large ... 951,155,955
            // reverse the number so that lower number has a "higher" Y coordinate value on the browser
            var delta2 = Math.floor ( Number ( delta1 / NetGfxDivisor[RxTxIndex][netIfIdx] ) );
            delta2 = Number(100 - delta2);
            if (objline) {
                var minYcoord = AddSvgPoints ( objline, delta2 );
                if (minYcoord > 90) { // big values dropped off ... adjust the Y scale ... > 90% of graph height is used
                    NetGfxDivisor[RxTxIndex][netIfIdx] = 1; // force the y-axis to redraw the next time around
                    ResetPolylinePoints ( objline, netIfIdx, RxTxIndex );
                    UpdateDashTickText ( "dash_" + RxTxStr[RxTxIndex] + "_" + netIfIdx, Number ( NetGfxDivisor[RxTxIndex][netIfIdx] * 100 ) );
                }
            }

            AppendToTextarea ( 'txt_netgfx_' + RxTxStr[RxTxIndex] + '_' + netIfIdx, ConvertBitsToMbps ( delta1 ) );

            // if this is the first time we are appending a value, the first value gets saved twice ( so that the graph doesn't start at zero)
            if ( NetGfxBytes[RxTxIndex][netIfIdx].length == 0 ) {
                AppendNetGfxBytes(RxTxIndex, netIfIdx, delta1 );
            }
            AppendNetGfxBytes(RxTxIndex, netIfIdx, delta1 );
        }
    }

    return true;
}

/**
 *  Function: This function will update the line green progress bar used to show how far
 *            the playback function has progressed.
 **/
function UpdateProgressGraph ( )
{
    var objprogressbar =document.getElementById("progressbar");
    var PlaybackArrayLen = Number ( RecordFileContentsArray.length - 1); // 0th entry is always blank
    var percentage = Math.ceil ( (PlaybackFileContentsArrayIdx / PlaybackArrayLen ) * 500 ) /* bar is 500 pixels long */;
    var points = "0,0 " + percentage + ",0 " + percentage + ",10 0,10";
    //console.log("state:" + PlaybackControl.Value + "; len:" + PlaybackArrayLen + "; idx:" + PlaybackFileContentsArrayIdx + "; points: " + points );
    if (objprogressbar) {
        objprogressbar.setAttribute('points', points);
    }
}

/**
 *  Function: This function will enable or disable ALL input buttons when a Playback
 *            is first selected and when the run/pause button is selected.
 **/
function PlaybackElementsSetDisabled ( newState )
{
    if ( PlaybackElementsDisabled.length ) { // if there is a list of elements we disabled for playback
        var elements = PlaybackElementsDisabled.split(",");
        for ( var idx=0; idx<elements.length; idx++ ) { // loop through all disabled elements and re-disable them
            setButtonDisabled ( elements[idx], newState );
        }
    }
}

/**
 *  Function: This function is the main handler when a playback is in progress. It will read the next
 *            playback record and process the playback record just as it would have been processed during
 *            regular live processing ... by calling ProcessResponses() ... which is also called during
 *            a normal response when the XMLHttpRequest comes back with an okay status.
 **/
function ProcessPlaybackEvents( )
{
    //console.log("ProcessPlaybackEvents: len " + RecordFileContentsArray.length + "; idx:" + PlaybackFileContentsArrayIdx );
    if ( RecordFileContentsArray.length && PlaybackFileContentsArrayIdx < RecordFileContentsArray.length ) {

        clearTimeoutOneSecond( "ProcessPlaybackEvents" );
        clearTimeout ( CgiRetryTimeoutId );

        var seconds_response = RecordFileContentsArray[PlaybackFileContentsArrayIdx].split("|!|");
        if ( seconds_response.length > 1 ) {
            var oResponses;
            //alert("seconds=" + seconds_response[0] + "; response(" + seconds_response[1] + ")" );
            oResponses = seconds_response[1].split("~");

            ProcessResponses ( oResponses );

            // If the oResponses included "netStatsInit", then the HTML table was re-created.
            // We need to re-disable user inputs.
            if ( PlaybackFileContentsArrayIdx == 1 ) {
                PlaybackElementsSetDisabled ( true );
            }

            UpdateCpuGraphs();

            UpdateProgressGraph();

            PlaybackFileContentsArrayIdx++;

            // make sure the export buttons are enabled
            setButtonDisabled( 'buttonExportCpuStats', false );
            setButtonDisabled( 'buttonExportNetStats', false );

            if ( PlaybackFileContentsArrayIdx >= RecordFileContentsArray.length ) { // playback is done
                PlaybackFileContentsArrayIdx = 0;
                PlaybackControl.Value = 0;
                //console.log( "ProcessPlaybackEvents; new state:" + PlaybackControl.Value );
                setButtonImage ( "buttonPlaybackRun", "bmemperf_play.jpg" );
                RecordPlaybackConfigure ( "STOP" );

                if ( PlaybackElementsDisabled.length ) { // if there is a list of elements we disabled for playback
                    PlaybackElementsSetDisabled ( false );
                }

                setButtonDisabled( 'checkboxRecord', false );
            }

            if ( PlaybackTimeoutId ) { clearTimeout( PlaybackTimeoutId ); }

            if ( PlaybackControl.Value ) {
                // schedule myself for processing one second from now
                PlaybackTimeoutId = setTimeout ('ProcessPlaybackEvents()', REFRESH_IN_MILLISECONDS );
            }
        }
    }
}

/**
 *  Function: This function will step through all of the responses that come back from the
 *            CGI call. Each response typically has special processing associated with each
 *            of them. This function is also called during Playback to simulate the same
 *            response processing.
 **/
function ProcessResponses ( oResponses )
{
    var debug=0;
    var i=0;
    var idx=0;
    var idx2=0;

    var row_profiling_html = document.getElementById("row_profiling_html");
    var objPerfTop = document.getElementById("PERFUTILS");
    if (row_profiling_html) {
        row_profiling_html.innerHTML = ""; // clear out the row
    }

    if(debug) console.log("ProcessResponses: for i = 0 to " + oResponses.length );
    // loop through <response> elements, and add each nested
    for ( i = 0; i < oResponses.length; i++) {
        var entry = oResponses[i];
        if ( entry.length>1 ) if(debug==1) console.log("Entry " + entry + "; next len " + oResponses[i+1].length + eol );
        if ( entry == "ALLDONE" ) {
            AddToDebugOutput ( entry + eol );
            ResponseCount++;

            if ( GetWifiStatsCountdown ) {
                GetWifiStatsCountdown--;
                if (GetWifiStatsCountdown == 0) {
                    GetWifiStats.Value = 1;
                }
            }

            //AddToDebugOutput ( entry + ": calling setTimeout(); ID (" + CgiTimeoutId + ")"  + eol );
        } else if (entry == "FATAL") {
            AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
            alert("FATAL ... " + oResponses[i+1]);
            i++;
        } else if ( entry == "DEBUG" ) {
            AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
            if(debug) alert(entry + ":" + oResponses[i+1]);
            i++;
        } else if (entry == "VERSION") {
            AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
            PlaybackVERSION = oResponses[i+1];
            i++;
        } else if (entry == "SPLICETIME") {
            PlaybackSTBTIME = oResponses[i+1];
            if( RecordControl.Value > 0 ) RecordFileContents +=  "~" + entry + "~" + oResponses[i+1];
            //AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
            var obj2=document.getElementById("splicetime");
            if (obj2) {
                //if (debug) alert("setting stbtime to " + oResponses[i+1] );
                obj2.innerHTML = oResponses[i+1];
                i++;
            }
        } else if (entry == "CPUPERCENTS") {
            AddToDebugOutput ( entry + ": len of entry " + oResponses[i+1].length + eol );
            PlaybackCPUPERCENTS = oResponses[i+1];
            if( RecordControl.Value > 0 ) RecordFileContents +=  "~" + entry + "~" + oResponses[i+1];
            var obj2=document.getElementById("CPUPERCENTS");
            if (obj2) {
                obj2.innerHTML = oResponses[i+1];
                //alert("CPUPERCENTS (" + obj2.innerHTML + ")" );
                i++;
            } else {
                alert("id=CPUPERCENTS not found");
            }
        } else if (entry == "CPUINFO") { // CPUINFO:4  99.97 100.0  100.0  100.0   99.98    0.1  ;
            if( RecordControl.Value > 0 ) RecordFileContents +=  "~" + entry + "~" + oResponses[i+1];
            var objCheckboxCpus = document.getElementById("checkboxcpus");
            if (objCheckboxCpus) {
                // if the checkbox to show CPU utilization is checked
                if (objCheckboxCpus.checked) {
                    var response = oResponses[i+1];
                    var cpupercentage = 0;
                    var tempNumCpus = 0;
                    var cpuUsageTotal = 0; // the four CPU usage's combined; used to compute average CPU usage
                    var cpuActiveCount = 0;

                    //AddToDebugOutput ( entry + ":" + response + ";" + eol );

                    // if the number of cpus has changed
                    tempNumCpus = Number(response.substr(0,1)) + 1;
                    if (tempNumCpus && tempNumCpus != gNumCpus ) {
                        var numlines = 0;
                        gNumCpus = tempNumCpus;
                        //alert("CPUINFO: gNumCpus " + gNumCpus );
                        for (numlines=1; numlines<(gNumCpus-1)+1 ; numlines++) { //once for all CPUs
                            showRow(numlines);
                        }
                    }

                    for (idx=0; idx < gNumCpus; idx++ ) {
                        var idle=response.substr(2+(idx*7), 7);

                        // if the string has some length of characters to parse
                        //alert("idx " + idx + "; idle " + idle + "; len " + idle.length );
                        if (idle.length) {
                            cpupercentage = Math.floor(idle); // each entry is 6 chars long ... CPUINFO:4 000100 000099 000255 000100
                            gCpuData[idx] = cpupercentage;
                            var cpuid="cpudata0" + idx;
                            //alert("cpu " + idx + "=(" + cpupercentage + "); idle (" + idle + "); cpuid " + cpuid + "; gNumCpus " + gNumCpus );
                            var cpudataobj = document.getElementById(cpuid);
                            if (cpudataobj) {
                                var cputitleid = "cputitle0" + idx;
                                var objtitle = document.getElementById(cputitleid);

                                ChangeCpuTag ( idx, idle );
                                if (idle == 255) { // if the cpu has been disabled
                                    // add inactive to the title
                                    if (objtitle) {
                                        objtitle.innerHTML = "CPU " + idx + "&nbsp;&nbsp;(<span style=\"background-color:red;\">(INACTIVE)</span>)";
                                    }
                                } else {
                                    var usage = Number(100 - idle);
                                    cpudataobj.innerHTML += usage + " ";

                                    if (objtitle) {
                                        var MHz = Number(gCpuFreq[idx]);
                                        if ( MHz == 0 || MHz == 999999 ) MHz = "__";
                                        objtitle.innerHTML = "CPU " + idx + "&nbsp;&nbsp;(&nbsp;" + usage + "% @ " + MHz + " MHz)";
                                    }

                                    cpuUsageTotal += Number(usage);
                                    cpuActiveCount++;
                                }

                                cpudataobj.scrollTop = cpudataobj.scrollHeight;
                                if (bCpuDataHeightSet == 0) {
                                    cpudataobj.style.height ='auto';
                                    cpudataobj.style.height ="18px";
                                }
                            }  // if cpudata00 is valid
                        } // if idle.length is valid
                    }  // for each CPU

                    cpuUsageLongAverage[cpuUsageLongAverageIdx] = (cpuUsageTotal / cpuActiveCount);
                    cpuUsageLongAverageIdx++; // increment index into array ... wrapping around to 0 if it gets too big
                    if ( cpuUsageLongAverageIdx >= CPU_USAGE_LONG_AVERAGE_MAX ) cpuUsageLongAverageIdx = 0;
                    cpuUsageLongAverageCount++; // Count is used to only compute average for 2 cycles if we have only been collecting values for 2 seconds

                    // update the total average CPU usage
                    var objcpuoverall = document.getElementById("cpuoverall");
                    if (objcpuoverall) {
                        objcpuoverall.innerHTML = Number( cpuUsageTotal / cpuActiveCount ).toFixed(1) + "%";
                        var avg = ComputeTotalCpuLongAverage();
                        objcpuoverall.innerHTML += "&nbsp;<span style=\"color:limegreen;\">(" + CPU_USAGE_LONG_AVERAGE_MAX + "s&nbsp;avg:" + avg + "%)</span>";
                        objcpuoverall.innerHTML += "<span style=\"font-size:8pt;\" >&nbsp;TOTAL&nbsp;CPU</span>";

                        // if there are two or more CPUs, update the red overall average line and the limegreen 5-second cpu average line
                        if (gNumCpus > 1) {
                            AppendPolylineXY ( 0, 100 - Math.floor ( Number( cpuUsageTotal / cpuActiveCount ) ) ); // update red CPU utilization average line
                            AppendPolylineXY ( gNumCpus, 100 - Math.floor ( Number( avg ) ) ); // line number needs to match the polyline0x definition in bsysperf.c
                                                                                               // update green 5-second CPU utilization average line
                        }
                    }

                    bCpuDataHeightSet = 1;
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "CPUFREQUENCY") {
            if( RecordControl.Value > 0 ) RecordFileContents +=  "~" + entry + "~" + oResponses[i+1];
            var response = oResponses[i+1];
            //AddToDebugOutput ( entry + ":" + response + ";" + eol );
            var objCheckboxCpus = document.getElementById("checkboxcpus");
            if (objCheckboxCpus) {
                // if the checkbox to show CPU utilization is checked
                if (objCheckboxCpus.checked) {
                    var tempNumCpus = 0;
                    tempNumCpus = Number(response.substr(0,1));
                    for (idx=0; idx < tempNumCpus; idx++ ) {
                        // 000000 means cpu is active but freq is unknown (old BOLT?); 999999 means cpu is inactive
                        var freq = rtrim ( response.substr(2+(idx*7), 7) );
                        //AddToDebugOutput ( "cpu " + idx + ":" + freq+ ";" + eol );
                        gCpuFreq[idx] = freq;
                    }  // for each CPU
                }
            }
            i++;
        } else if (entry == "ETHINFO") {
            var response = oResponses[i+1];
            AddToDebugOutput ( entry + ":" + response + ";" + eol );
            i++;
        } else if (entry == "IRQDETAILS") {
            var response = oResponses[i+1];
            if( RecordControl.Value > 0 ) RecordFileContents +=  "~" + entry + "~" + oResponses[i+1];
            var obj2=document.getElementById("IRQDETAILS");
            if (obj2) {
                obj2.innerHTML = oResponses[i+1];
            }
            i++;
        } else if (entry == "MEMORY") {
            var obj2 = document.getElementById("checkboxmemory");
            if (obj2) {
                if (obj2.checked) {
                    var response = oResponses[i+1];
                    var obj2=document.getElementById("MEMORY");
                    if (obj2) {
                        obj2.innerHTML = oResponses[i+1];
                    }
                    validResponseReceived = 1;
                    hideOrShow("row_memory", SHOW );
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "netStatsInit") { // only return when user first checks the Network Stats checkbox
            PlaybacknetStatsInit = oResponses[i+1];
            if( RecordControl.Value > 0 ) RecordFileContents +=  "~" + entry + "~" + oResponses[i+1];
            var obj2 = document.getElementById("checkboxnets");
            if (obj2) {
                if (obj2.checked) {
                    var response = oResponses[i+1];
                    var obj2=document.getElementById("NETSTATS");
                    if (obj2) {
                        obj2.innerHTML = oResponses[i+1];
                        AddToDebugOutput ( entry + ":iperfClientServerRow:" + iperfClientServerRow + eol );
                    }

                    // hide all ten of the histogram rows because the visibility:hidden attribute in the HTML doesn't work initially
                    for ( var idx=0; idx<10; idx++) {
                        hideOrShow("row_netgfxsvg" + idx, HIDE );
                        hideOrShow("row_netgfxtxt" + idx, HIDE );
                    }

                    // if we are doing a playback
                    if ( PlaybackFileContentsArrayIdx ) {
                        // when the user selects a file for playback, we need to SHOW any graphs that are selected
                        for ( var idx=0; idx<10; idx++) {
                            var obj = document.getElementById ( "checkbox_netgfx" + idx );
                            if ( obj && NetGfxCheckbox[idx] ) {
                                hideOrShow("row_netgfxsvg" + idx, SHOW );
                                hideOrShow("row_netgfxtxt" + idx, SHOW );
                            }
                        }
                    }
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;

        } else if (entry == "netStatsUpdate") {
            if(debug) console.log ( entry + ":" + oResponses[i+1] + eol );
            if( RecordControl.Value > 0 ) RecordFileContents +=  "~" + entry + "~" + oResponses[i+1];
            i++;
            var num_interfaces = oResponses[i];
            i++;
            for ( var idx=0; idx<num_interfaces; idx++ ) {
                if( RecordControl.Value > 0 ) RecordFileContents +=  "~" + oResponses[i];
                var splits = oResponses[i].split('|'); // rx_bytes|tx_bytes|rx_error|tx_error
                if(debug) console.log ( entry + ":" + idx + " - " + oResponses[i] + ": splits.length=" + splits.length + eol );
                if ( splits.length == 4 ) {
                    var objcell = "";
                    var tagid = "";
                    for ( var jdx=0; jdx<splits.length; jdx++ ) {
                        if ( jdx==0 ) {
                            tagid = "netif_rxBytes_" + idx;
                        } else if ( jdx==1 ) {
                            tagid = "netif_txBytes_" + idx;
                        } else if ( jdx==2 ) {
                            tagid = "netif_rxError_" + idx;
                        } else if ( jdx==3 ) {
                            tagid = "netif_txError_" + idx;
                        }
                        objcell = document.getElementById( tagid );
                        if (objcell) {
                            objcell.innerHTML = splits[jdx];
                        }
                    }
                }
                i++;
                if(debug) console.log ( "netif " + idx + ": next response to process ... " + oResponses[i] + eol );
            }

            // after loop above finishes, the index "i" is pointing to the wrong entry
            i--;

            // because the checkboxbox gets updated every second, we have to remember the status of the previous second
            if ( iperfClientServerRow == 1 ) { // if the previous status was checked, set current status to same
                var objbox=document.getElementById('checkboxiperfrow');
                if (objbox) {
                    objbox.checked = true;
                }
                hideOrShow("row_iperf_client_server", SHOW );
            } else {
                hideOrShow("row_iperf_client_server", HIDE );
            }

            if ( iperfStateClient != iperfStateEnum.UNINIT || iperfStateServer != iperfStateEnum.UNINIT ) {
                set_iperf_count_cell();
                set_iperf_status_cell();
            }

        } else if (entry == "NETBYTES") {
            if( RecordControl.Value > 0 ) RecordFileContents +=  "~" + entry + "~" + oResponses[i+1];
            var obj2 = document.getElementById("checkboxnets");
            var Mbps = 0;
            var Bits = 0;
            if (obj2) {
                if (obj2.checked) {
                    if(debug) console.log ( entry + ":" + oResponses[i+1] + eol );
                    NetBytesSeconds++;

                    var response = oResponses[i+1];
                    if(debug) console.log ( entry + ":" + response + "; NetBytesSeconds " + NetBytesSeconds + eol );
                    var oRxTxPairs = response.split( "," ); // split the response using comma delimiter

                    for (var idx = 0; idx < oRxTxPairs.length; idx++) {
                        if(debug) console.log ( "NetIF " + idx + "str(" + oRxTxPairs[idx] + ")" + eol );
                        if (idx < 10 && oRxTxPairs[idx].length >= 3 ) { // if we haven't exceeded array size and rxtx pair has some values
                            var oRxTx = oRxTxPairs[idx].split( " " ); // split the response using space as delimiter
                            var tagRx = "netif" + idx + "rx";
                            var objRx = document.getElementById(tagRx);
                            var tagTx = "netif" + idx + "tx";
                            var objTx = document.getElementById(tagTx);

                            if(debug) console.log( entry + ": objRx (" + objRx + ") objTx (" + objTx + ")" );
                            if (! objRx ) {
                                continue;
                            }

                            if (! objTx ) {
                                continue;
                            }

                            // instead of multiplying by 8 and then dividing by 1024 ... reduce it simply to dividing by 128
                            Mbps =  Number( (oRxTx[0] - NetBytesPrev[idx][0]) / 128 / 1024);
                            Bits =  Number( (oRxTx[0] - NetBytesPrev[idx][0]) * 8 );

                            if (NetBytesPrev[idx][0] > 0 && (oRxTx[0] >= NetBytesPrev[idx][0]) && (NetBytesSeconds > 1) && (Mbps > 0) ) {
                                // if the array has filled up
                                if (NetBytesRx10SecondsCount[idx] == 10) {
                                    for (idx2=0; idx2<(NetBytesRx10SecondsCount[idx]-1); idx2++) {
                                        NetBytesRx10Seconds[idx][idx2] = NetBytesRx10Seconds[idx][idx2+1];
                                    }
                                    NetBytesRx10SecondsCount[idx] = 9;
                                }
                                NetBytesRx10Seconds[idx][NetBytesRx10SecondsCount[idx]] = Mbps;
                                NetBytesRx10SecondsCount[idx]++;
                                if (objRx) {
                                    // NetBytesSeconds is subtracted by one because we need at least two-second's worth of data to compute a delta
                                    // convert bytes to megabits
                                    // instead of multiplying by 8 and then dividing by 1024 ... reduce it simply to dividing by 128
                                    objRx.innerHTML = ConvertBitsToMbps ( Bits ) + "&nbsp;&nbsp;(" + ComputeNetBytes10SecondsAverage ( idx, 0 ) + ")"; // 0 for RX and 1 for TX
                                }
                            } else {
                                objRx.innerHTML = "0.000 (0.000)";
                            }
                            UpdateNetGfxElements ( idx, 0/*RX*/, NetBytesPrev[idx][0], oRxTx[0] );
                            NetBytesPrev[idx][0] = oRxTx[0];

                            // instead of multiplying by 8 and then dividing by 1024 ... reduce it simply to dividing by 128
                            Mbps =  Number( (oRxTx[1] - NetBytesPrev[idx][1]) / 128 / 1024);
                            Bits =  Number( (oRxTx[1] - NetBytesPrev[idx][1]) * 8 );

                            if (NetBytesPrev[idx][1] > 0 && (oRxTx[1] >= NetBytesPrev[idx][1]) && (NetBytesSeconds > 1) && (Mbps > 0) ) {
                                // if the array has filled up, move the entries left one position and add new entry to the very end
                                if (NetBytesTx10SecondsCount[idx] == 10) {
                                    for (idx2=0; idx2<(NetBytesTx10SecondsCount[idx]-1); idx2++) {
                                        NetBytesTx10Seconds[idx][idx2] = NetBytesTx10Seconds[idx][idx2+1];
                                    }
                                    NetBytesTx10SecondsCount[idx] = 9;
                                }
                                NetBytesTx10Seconds[idx][NetBytesTx10SecondsCount[idx]] = Mbps;
                                NetBytesTx10SecondsCount[idx]++;
                                if (objTx) {
                                    // NetBytesSeconds is subtracted by one because we need at least two-second's worth of data to compute a delta
                                    // convert bytes to megabits
                                    // instead of multiplying by 8 and then dividing by 1024 ... reduce it simply to dividing by 128
                                    objTx.innerHTML = ConvertBitsToMbps ( Bits ) + "&nbsp;&nbsp;(" + ComputeNetBytes10SecondsAverage ( idx, 1 ) + ")"; // 0 for RX and 1 for TX
                                }
                            } else {
                                objTx.innerHTML = "0.000 (0.000)";
                            }
                            UpdateNetGfxElements ( idx, 1/*TX*/, NetBytesPrev[idx][1], oRxTx[1] );
                            NetBytesPrev[idx][1] = oRxTx[1];
                        }
                    }
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "iperfInit") {
            //alert(entry + " is (" + oResponses[i+1] + ")" );
            var obj2 = document.getElementById("checkboxnets");
            if (obj2) {
                if (obj2.checked) {
                    var response = oResponses[i+1];
                    var obj2=document.getElementById("iperfInit");
                    if (obj2) {
                        obj2.innerHTML = oResponses[i+1];
                    }
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "iperfPidClient") {
            var obj2 = document.getElementById("checkboxnets");
            if (obj2) {
                if (obj2.checked) {
                    iperfPidClient = oResponses[i+1];
                    if (iperfPidClient == 0 && iperfStateClient == iperfStateEnum.RUNNING) {
                        //alert(entry + " is (" + oResponses[i+1] + ")" );
                        iperfStateClient = iperfStateEnum.STOP;
                        set_iperf_button( entry );
                        iperfPidClient = "terminated";
                    }
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "iperfPidServer") {
            //alert(entry + " is (" + oResponses[i+1] + ")" );
            var obj2 = document.getElementById("checkboxnets");
            if (obj2) {
                if (obj2.checked) {
                    iperfPidServer = oResponses[i+1];
                    if (iperfPidServer == 0 && iperfStateServer == iperfStateEnum.RUNNING) {
                        // The first time after a reboot when we try to start the server, the events happen so slowly that the returned
                        // PID is 0. This zero PID was causing the logic to change states to STOP inadvertently. By adding a 1-second
                        // retry, the premature abort after reboot went away.
                        if ( iperfStateServerRetry < 1 ) {
                            iperfStateServerRetry++;
                        } else {
                            iperfStateServerRetry=0;
                            iperfStateServer = iperfStateEnum.STOP;
                            set_iperf_button( entry );
                            iperfPidServer = "terminated";
                        }
                    }
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "iperfErrorClient") {
            //alert(entry + " is (" + oResponses[i+1] + ")" );
            var obj2 = document.getElementById("checkboxnets");
            if (obj2) {
                if (obj2.checked) {
                    iperfPidClient = oResponses[i+1];
                    iperfStateClient = iperfStateEnum.UNINIT;
                    set_iperf_button( entry );
                    set_iperf_status_cell();
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "BACKGROUND") {
        		document.querySelector('body').style.backgroundColor = "yellow";
        } else if (entry == "iperfErrorServer") {
            //alert(entry + " is (" + oResponses[i+1] + ")" );
            var obj2 = document.getElementById("checkboxnets");
            if (obj2) {
                if (obj2.checked) {
                    iperfPidServer = oResponses[i+1];
                    iperfStateServer = iperfStateEnum.UNINIT;
                    iperfStateServerRetry = 0;
                    set_iperf_button( entry );
                    set_iperf_status_cell();
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "iperfRunningClient") {
            //console.log (entry + " is (" + oResponses[i+1] + ") ... len " + oResponses[i+1].length);
            var obj2 = document.getElementById("checkboxnets");
            if (obj2) {
                if (obj2.checked) {
                    if ( oResponses[i+1].length ) {
                        if ( oResponses[i+1] == "NONE" ) { // if there is no iperf -c running, make sure the START button is enabled
                            //set_iperf_cmd ( "", entry ); // if we clear this out, what user is typing gets cleared BEFORE START button pressed
                            setButtonDisabled( 'iperf_start_stop_c', false );
                        } else {
                            iperfRunningClient = oResponses[i+1];

                            //fillin_iperf_entry_boxes();
                            var obj=document.getElementById("iperf_start_stop_c");
                            if (obj && obj.value == "START") {
                                obj.value = "STOP";
                                iperfStateClient = iperfStateEnum.RUNNING;
                                set_iperf_cmd ( iperfRunningClient.substr(iperfRunningClient.indexOf('iperf ')), entry );
                                set_iperf_count_value(entry, get_unix_seconds( iperfRunningClient ) ); // if we loaded the page and iperf is already running, set the start time
                            }
                            setButtonDisabled( 'iperf_start_stop_c', true ); // will be enabled if browser received iperfClientRate
                        }
                    }
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "iperfRunningServer") {
            //console.log ( entry + " is (" + oResponses[i+1] + ")" );
            var obj2 = document.getElementById("checkboxnets");
            if (obj2) {
                if (obj2.checked) {
                    iperfRunningServer = oResponses[i+1];
                    if (iperfRunningServer.length) {
                        //fillin_iperf_entry_boxes();
                        var obj=document.getElementById("iperf_start_stop_s");
                        if (obj && obj.value == "START") {
                            // if the running iperf was started by bsysperf (i.e. it has the special -Q option)
                            if ( oResponses[i+1].indexOf(" iperf -s -Q ") > 0 ) {
                                obj.value = "STOP";
                                iperfStateServer = iperfStateEnum.RUNNING;
                                set_iperf_cmd ( iperfRunningServer.substr(iperfRunningServer.indexOf('iperf ')), entry );
                                set_iperf_count_value(entry, get_unix_seconds( iperfRunningServer ) ); // if we loaded the page and iperf is already running, set the start time
                            } else {
                                if ( oResponses[i+1] == "NONE" ) { // if there is no iperf -s running, make sure the START button is enabled
                                    setButtonDisabled( 'iperf_start_stop_s', false );
                                    //set_iperf_cmd ( "", entry ); // if we clear this out, what user is typing gets cleared BEFORE START button pressed
                                } else {
                                    // if someone started iperf -s outside of bsysperf, do not allow bsysperf to start another one
                                    setButtonDisabled( 'iperf_start_stop_s', true );
                                    set_iperf_cmd ( iperfRunningServer.substr(iperfRunningServer.indexOf('iperf ')) + " (started outside of bsysperf)", entry );
                                }
                            }
                        }
                    }
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "iperfClientRate") {
            //console.log( entry + " is (" + oResponses[i+1] + ")" + eol );
            iperfClientRate = oResponses[i+1];
            setButtonDisabled( 'iperf_start_stop_c', false );
            i++;
        } else if (entry == "WIFIINIT") {
            var obj2 = document.getElementById("checkboxwifi");
            if (obj2) {
                if (obj2.checked) {
                    //alert( entry + " - (" + oResponses[i+1] + ")" );
                    var response = oResponses[i+1];
                    var obj2=document.getElementById("WIFIINIT");
                    if (obj2) {
                        obj2.innerHTML = oResponses[i+1];
                        //alert("responses added to element WIFIINIT");
                    }
                    GetWifiStatsCountdown = 10;
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                    //alert("checkboxwifi element not checked");
                }
            } else {
                alert("checkboxwifi element not found ");
            }
            i++;
        } else if (entry == "WIFISTATS") {
            var obj2 = document.getElementById("checkboxwifi");
            if (obj2) {
                if (obj2.checked) {
                    //alert( entry + " - (" + oResponses[i+1] + ")" );
                    var obj3=document.getElementById("WIFISTATS");
                    if (obj3) {
                        obj3.innerHTML = oResponses[i+1];
                        GetWifiStats.Value = 0;
                    }
                    GetWifiStatsCountdown = 10;
                } else {
                    var local = new Date();
                    var Seconds = Math.floor(local.getTime() / 1000);
                    AddToDebugOutput ( entry + ": SecondsEnabled:" + GetWifiStats.SecondsEnabled + " ... delta ... " + (Seconds - GetWifiStats.SecondsEnabled ) + eol );
                    if ( GetWifiStats.SecondsEnabled ) {
                        if ( (Seconds - GetWifiStats.SecondsEnabled ) > 10 ) {
                            setButtonDisabled( 'checkboxwifi', false );
                        }
                    }
                }
            }
            i++;
        } else if (entry == "WIFIRATE") {
            var obj2 = document.getElementById("checkboxwifi");
            if (obj2) {
                if (obj2.checked) {
                    //alert( entry + " - (" + oResponses[i+1] + ")" );
                    var obj3=document.getElementById("WIFIRATE");
                    if (obj3) {
                        obj3.innerHTML = oResponses[i+1];
                    }
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "WIFIDISABLED") {
            //alert( entry );
            var obj2 = document.getElementById("checkboxwifi");
            if (obj2) {
                obj2.disabled = true;
                obj2.checked = false;
                GetWifiStats.SecondsEnabled = 0;
                hideOrShow("row_wifi_stats", HIDE );
                hideOrShow("row_wifi_ampdu", HIDE );
            }
        } else if (entry == "WIFIENABLED") {
            //alert( entry );
            AddToDebugOutput ( entry + ": SecondsEnabled:" + GetWifiStats.SecondsEnabled + eol );
            var local = new Date();
            if ( GetWifiStats.SecondsEnabled == 0) {
                GetWifiStats.SecondsEnabled = Math.floor(local.getTime() / 1000);
            }
        } else if (entry == "WIFISCANMAXAPS") {
            // The interface with bsysperf_server can accommodate a certain number of access points; this is the maximum it can handle each second
            // If the max is 8 and we have 12 APs to send back to client, ask for 1st set of 8, and then ask for 2nd set of 8 of which only 4 will be returned
            if ( oResponses[i+1] ) {
                //alert( entry + "-" + oResponses[i+1] );
                GetWifiScan.MaxNumAps = oResponses[i+1];
            }
            i++;
        } else if (entry == "WIFISCANNUMAPS") {
            //alert( entry + "-" + oResponses[i+1] );
            var obj2 = document.getElementById("checkboxwifi");
            if (obj2 && obj2.checked ) {
                //alert(entry + "-" + oResponses[i+1] );
                if ( Number(oResponses[i+1]) < Number(GetWifiScan.MaxNumAps) ) {
                    // if the number of APs returned from the server is less than the maximum, we have reached the end of the number of APs
                    GetWifiScan.State = GetWifiScanState.UNINIT;
                    setButtonDisabled ( "WifiScan", false );
                } else {
                    // we have transferred 8 of 12 APs; increment the starting index from 0 to 8 and get the last 4
                    GetWifiScan.ServerIdx = Number(GetWifiScan.ServerIdx) + Number(GetWifiScan.MaxNumAps);
                    AddToDebugOutput ( entry + ": GetWifiScan.ServerIdx:" + GetWifiScan.ServerIdx + eol );
                    //alert( entry + "- ServerIdx =" + GetWifiScan.ServerIdx );
                }
            }
            i++;
        } else if (entry == "WIFISCANRESULTS") {
            var obj2 = document.getElementById("checkboxwifi");
            if (obj2 && obj2.checked ) {
                //alert(entry + "-" + oResponses[i+1] );
                AddToHtml ( "WIFISCANRESULTS", oResponses[i+1] );
            }
            i++;
        } else if (entry == "wifiAmpduGraph") {
            var obj2 = document.getElementById("checkboxwifi");
            if (obj2 && obj2.checked ) {
                var objampdu = document.getElementById("checkboxWifiAmpduGraph");
                if (objampdu && objampdu.checked ) {
                    var objdiv=document.getElementById('SVG_DATA_ARRAY');
                    if (objdiv) {
                        var element_id = "";
                        //alert(entry + "-" + oResponses[i+1] );
                        objdiv.innerHTML = oResponses[i+1];

                        // the table may have some rows that are hidden based on the antenna strength
                        for (var idx=0; idx<8; idx++) {
                            element_id = "r_x" + idx;
                            var objrow = document.getElementById( element_id );
                            if (objrow) {
                                if (objrow.style.visibility == "hidden" ) {
                                    //alert("row " + element_id + " is hidden");
                                    hideOrShow ( element_id, HIDE );
                                }
                            }
                        }
                        for (var idx=0; idx<8; idx++) {
                            element_id = "R_x" + idx;
                            var objrow = document.getElementById( element_id );
                            if (objrow) {
                                if (objrow.style.visibility == "hidden" ) {
                                    //alert("row " + element_id + " is hidden");
                                    hideOrShow ( element_id, HIDE );
                                }
                            }
                        }

                        hideOrShow("row_wifi_ampdu", SHOW );
                        DrawScene(0);
                        SwapTxRx(0);
                        DrawScene(1);
                        SwapTxRx(1);
                        RadioClick(); // reset sidx based on the radio button selected
                    }
                } else {
                    //alert(entry + ": checkboxWifiAmpduGraphnot checked");
                }
            }
            i++;
        } else if (entry == "IRQINFO") {
            var obj2 = document.getElementById("checkboxirqs");
            if (obj2) {
                if (obj2.checked) {
                    var response = oResponses[i+1];
                    var irqcount = 0;
                    var tempNumCpus = Number(response.substr(0,1)) + 1;
                    //alert("IRQINFO: tempNumCpus " + tempNumCpus );
                    AddToDebugOutput ( entry + ":" + response + "; tempNumCpus " + tempNumCpus + ";" + eol );
                    for (idx=0; idx < tempNumCpus; idx++ ) {
                        var idle=response.substr(2+(idx*7), 7);

                        // if the string has some length of characters to parse
                        if (idle.length) {
                            gIrqLatestData[idx] = idle;
                            var irqid="irqdata0" + idx;
                            //alert("irq " + idx + "; idle (" + idle + "); irqid " + irqid + "; tempNumCpus " + tempNumCpus );
                            var irqdataobj = document.getElementById(irqid);
                            if (irqdataobj) {
                                var usage = Number(idle);
                                irqdataobj.innerHTML += usage + " ";

                                irqdataobj.scrollTop = irqdataobj.scrollHeight;
                                if (bIrqDataHeightSet == 0) {
                                    irqdataobj.style.height ='auto';
                                    irqdataobj.style.height ="18px";
                                }
                            }
                        }
                    }
                    bIrqDataHeightSet = 1;
                } else {
                    AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                }
            }
            i++;
        } else if (entry == "WEBVERSION") {
            PlaybackPLATFORM = oResponses[i+1];
            var objplatform = document.getElementById("webversion");
            if (objplatform) {
                objplatform.innerHTML = oResponses[i+1];
            }
            window.document.title = "SPLICE CENTERING SYSTEM " + oResponses[i+1];
			i++;
        } else if (entry == "SPLICEVERSION") {
            PlaybackVARIANT = oResponses[i+1];
            var objvariant = document.getElementById("spliceversion");
            if ( objvariant ) {
                objvariant.innerHTML = oResponses[i+1];
            }
            i++;
        } else if (entry == "CAMERAVERSION") {
            PlaybackPLATVER = oResponses[i+1];
            var objplatform = document.getElementById("cameraversion");
            if (objplatform) {
                objplatform.innerHTML = oResponses[i+1]
            }
            i++;
        } else if (entry == "UNAME") {
            PlaybackUNAME = oResponses[i+1];
            //alert(entry + ":" + oResponses[i+1] );
            var objplatform = document.getElementById("UNAME");
            if (objplatform) {
                objplatform.innerHTML = oResponses[i+1];
            }
            i++;
        } else if (entry == "MWIDTH") {
            var obj = document.getElementById("MWidth_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "SWIDTHIN") {
            var obj = document.getElementById("SWidthIn_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "OFFSETIN") {
            var obj = document.getElementById("OffsetIn_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "GETSWIDTH") {
            var obj = document.getElementById("GetSWidth_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "TOLPOS") {
            var obj = document.getElementById("TolPos_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "TOLNEG") {
            var obj = document.getElementById("TolNeg_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "SWIDTHOUT") {
            var obj = document.getElementById("SWidthOut_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "OFFSETOUT") {
            var obj = document.getElementById("OffsetOut_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "POFFSET") {
            var obj = document.getElementById("P_Offset_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "MOFFSET") {
            var obj = document.getElementById("M_Offset_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
/*
        } else if (entry == "LOENABLE") {
            var obj = document.getElementById("MWidth_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "TOENABLE") {
            var obj = document.getElementById("MWidth_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
*/
        } else if (entry == "LO0") {
            var obj = document.getElementById("LeadOffset0_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "LO1") {
            var obj = document.getElementById("LeadOffset1_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "LO2") {
            var obj = document.getElementById("LeadOffset2_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "LO3") {
            var obj = document.getElementById("LeadOffset3_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "LO4") {
            var obj = document.getElementById("LeadOffset4_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "LO5") {
            var obj = document.getElementById("LeadOffset5_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "LO6") {
            var obj = document.getElementById("LeadOffset6_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "TO0") {
            var obj = document.getElementById("TrailOffset0_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "TO1") {
            var obj = document.getElementById("TrailOffset1_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "TO2") {
            var obj = document.getElementById("TrailOffset2_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "TO3") {
            var obj = document.getElementById("TrailOffset3_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "TO4") {
            var obj = document.getElementById("TrailOffset4_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "TO5") {
            var obj = document.getElementById("TrailOffset5_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "TO6") {
            var obj = document.getElementById("TrailOffset6_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "ACTRESET") {
            var obj = document.getElementById("ActReset_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "ACTDIR") {
            var obj = document.getElementById("direction_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "ACTSTROKE") {
            var obj = document.getElementById("stroke_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "ACTSPEED") {
            var obj = document.getElementById("speed_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "ACTLLIMIT") {
            var obj = document.getElementById("left_limit_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "ACTRLIMIT") {
            var obj = document.getElementById("right_limit_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "ACTORG") {
            var obj = document.getElementById("origin_val");
            if (obj) {
                obj.value = oResponses[i+1];
            }
            i++;
        } else if (entry == "SERVERIP") {
            PlaybackPLATVER = oResponses[i+1];
            var objplatform = document.getElementById("serverip");
            if (objplatform) {
                objplatform.innerHTML = oResponses[i+1]
            }
            i++;
        } else if (entry == "CURVAL") {
            actCurPos = oResponse[i+1];
            i++;
        } else if (entry == "PREVVAL") {
            actPrevPos = oResponses[i+1];
            i++;
        } else if (entry == "BOLTVER") {
            PlaybackBOLTVER = oResponses[i+1];
            var objplatform = document.getElementById("BOLTVER");
            if (objplatform) {
                objplatform.innerHTML = "Bolt: " + oResponses[i+1];
            }
            i++;
        } else if (entry == "HEAPTABLE") { // used to populate the MEMORY_HTML TH with heap table and PerfCache
            AddToDebugOutput ( entry + ": len " + oResponses[i+1].length + ";" + eol );
            //alert(entry + ":" + oResponses[i+1] );

            var objtable = document.getElementById("MEMORY_HTML");
            if (objtable) {
                if (oResponses[i+1].indexOf("id=textareaPerfCache") > 0 ) { // if the response is the html for the Cache textbox
                    if ( GetCheckboxStatus ( "checkboxPerfCache" ) ) { // only display the response if the checkbox is still checked
                        objtable.innerHTML = oResponses[i+1];

                        objtable = document.getElementById("textareaPerfCache");
                        if (objtable) {
                            objtable.rows = Number(objtable.scrollHeight/14,0) + 1;
                            //alert("TEXTAREA height (" + objtable.scrollHeight + "); numrows (" + objtable.rows + ")" );
                        }
                    }
                } else {
                    objtable.innerHTML = oResponses[i+1];
                }
            }

            i++;
        } else if (entry == "HEAPINFO") {
            AddToDebugOutput ( entry + ": len " + oResponses[i+1].length + ";" + eol );
            //alert(entry + ":" + oResponses[i+1] );
            var objheapinfo = document.getElementById("heapinfo");
            if (objheapinfo) {
                objheapinfo.innerHTML = MsIeCrFix ( oResponses[i+1] );
            } else {
                //alert("element heapinfo not found");
            }
            AddToDebugOutput ( entry + ": len1:" + oResponses[i+1].length + "; " + oResponses[i+1] + eol );

            if (oResponses[i+1].length > 0) {
                validResponseReceived = 1;
                GetHeapStats.Value = 0;
                SetCheckboxStatus ( "checkboxPerfDeep", GetHeapStats );
                hideOrShow("row_memory", SHOW );
            } else {
                if (objheapinfo) {
                    //objheapinfo.innerHTML = "Response was invalid; trying again in 1 second!";
                }
            }
            i++;
        } else if (entry == "HEAPGRAPH") {
            AddToDebugOutput ( entry + ": len " + oResponses[i+1].length + ";" + eol );
            //alert(entry + ":" + oResponses[i+1] );

            var objplatform = document.getElementById("heapgraph");
            if (objplatform) {
                objplatform.innerHTML = oResponses[i+1]; CurrentPlatform = oResponses[i+1];
            } else {
                //alert("element heapgraph not found");
            }
            AddToDebugOutput ( entry + "[1]: len2:" + oResponses[i+1].length + "; " + oResponses[i+1] + eol );
            i++;
        } else if (entry == "SATADEBUG") {
            AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
            i++;
        } else if (entry == "SATAUSB") {
            //alert(entry + ":" + oResponses[i+1]);
            AddToDebugOutput ( entry + ": html table len " + oResponses[i+1].length + ";" + eol );

            var objcheckbox = document.getElementById("checkboxsatausb");
            var objtable = document.getElementById("MEMORY_HTML");
            if (objcheckbox) {
                objcheckbox.checked = GetSataUsb.Value;
            }
            //alert("got SATAUSB; GetSataUsb is " + GetSataUsb.Value );
            // if the checkbox is still checked, populate the information
            if (GetSataUsb.Value) {
                if (objtable) { objtable.innerHTML = oResponses[i+1]; }
            } else {
                if (objtable) { objtable.innerHTML = ""; }
                //alert("got SATAUSB; cleared innerHTML" );
            }
            i++;
        } else if (entry == "ALERT") {
            alert( oResponses[i+1] );
            i++;
        } else if (entry == "PERFDEEPSTARTED" && GetCheckboxStatus ( "checkboxprofiling" ) && GetCheckboxStatus ( "checkboxPerfDeep" ) ) {
            AddToDebugOutput ( entry + ": response (" + oResponses[i+1] + "); " + eol );
            //alert ( entry + ": response (" + oResponses[i+1] + "); " + eol );
            if (oResponses[i+1] == "SUCCESS") {
                GetPerfDeepCountdown = GetPerfDeep.Duration;
                GetPerfDeepCountdown++;
                //alert("Perf Record succeeded; setting timeout for  " + GetPerfDeepCountdown );
                if ( GetPerfDeepCountdown > 1) {
                    //alert( entry + "; duration " + GetPerfDeepCountdown );
                    setTimeout ('GetPerfDeepResultsDoit()', 1000 );
                }
            } else {
                alert("Perf Record failed: " + oResponses[i+1] );
            }
            i++;
        } else if (entry == "PERFCACHESTARTED" && GetCheckboxStatus ( "checkboxmemory" ) && GetCheckboxStatus ( "checkboxPerfCache" ) ) {
            AddToDebugOutput ( entry + ": response (" + oResponses[i+1] + "); " + eol );
            //alert ( entry + ": response (" + oResponses[i+1] + "); " + eol );
            if (oResponses[i+1] == "SUCCESS") {
                GetPerfCacheCountdown = GetPerfCache.Duration;
                GetPerfCacheCountdown++;
                //alert("Perf stat succeeded; setting timeout for  " + GetPerfCacheCountdown );
                if ( GetPerfCacheCountdown > 1) {
                    //alert( entry + "; duration " + GetPerfCacheCountdown );
                    setTimeout ('GetPerfCacheResultsFunc()', 1000 );
                }
            } else {
                alert("Perf Stat failed: " + oResponses[i+1] );
            }
            i++;
        } else if (entry == "PERFDEEPRESULTSDONE") {
            AddToDebugOutput ( entry + eol );
            SetCheckboxStatus ( "checkboxPerfDeep", GetPerfDeep );
            //alert( entry + ": GetPerfDeep.Value " + GetPerfDeep.Value );
            if ( GetCheckboxStatus ( "checkboxprofiling" ) && GetCheckboxStatus ( "checkboxPerfDeep" ) ) {
                checkboxPerfDeepDoit( true );
                //alert("report done; doing it again");
                GetPerfDeep.StartReportNow = true;
                //sendCgiRequest();
            }
        } else if (entry == "PERFCACHEDONE") {
            AddToDebugOutput ( entry + eol );
            SetCheckboxStatus ( "checkboxPerfCache", GetPerfCache );
            //alert( entry + ": GetPerfCache.Value " + GetPerfCache.Value );
            if ( GetCheckboxStatus ( "checkboxmemory" ) && GetCheckboxStatus ( "checkboxPerfCache" ) == true ) {
                checkboxPerfCacheDoit( true );
                //alert("stat done; doing it again");
                GetPerfCache.StartReportNow = true;
                //sendCgiRequest();
            }
        } else if (entry == "PERFENABLED") {
            AddToDebugOutput ( entry + eol );

            PerfError = false;
        } else if (entry == "PERFERROR") {
            //AddToDebugOutput ( entry + eol );

            PerfError = true;

            GetPerfTop.Value = 0;
            SetCheckboxStatus ( "checkboxPerfTop", GetPerfTop );
            SetCheckboxDisabled ( "checkboxPerfTop", true );

            GetPerfDeep.Value = 0;
            SetCheckboxStatus ( "checkboxPerfDeep", GetPerfDeep );
            SetCheckboxDisabled ( "checkboxPerfDeep", true );

            GetPerfCache.Value = 0;
            SetCheckboxStatus ( "checkboxPerfCache", GetPerfCache );
            SetCheckboxDisabled ( "checkboxPerfCache", true );

            SetCheckboxDisabled ( "checkboxLinuxTop", false );

            //alert("PERFERROR done");
        } else if ( (entry == "PerfTop") || (entry == "LinuxTop") || (entry == "PERFDEEPRESULTS") ) {
            //alert("Response " + entry + "; len=" + oResponses[i+1].length + ":" + oResponses[i+1] );
            CountdownSeconds = 0;
            PerfCacheCountdownSeconds = 0;
            if ( objPerfTop ) {
                if (entry == "LinuxTop" ) {
                    // sometimes the LinuxTop command comes back "empty" ... meaning there is a bunch of html that contains nothing useful
                    if ( oResponses[i+1].length > 1000 ) {
                        if (GetLinuxTop.Value) { // if LinuxTop checkbox is checked (we are expecting LinuxTop data)
                            objPerfTop.innerHTML = MsIeCrFix ( oResponses[i+1] );
                        }

                        // these have to be after the setting of innerHTML; otherwise the checkboxes won't be defined yet
                        SetCheckboxDisabled ( "checkboxPerfTop", GetLinuxTop.Value ); // if LinuxTop is true -> disable PerfTop; else Enable it
                        SetCheckboxDisabled ( "checkboxPerfDeep", GetLinuxTop.Value ); // if LinuxTop is true -> disable PerfDeep
                    } else { // the LinuxTop command failed to provide data, try again in this many milliseconds
                        setTimeout ('RetryLinuxTop()', 500 );
                    }
                } else {
                    if ( (entry == "PerfTop") ) {
                        objPerfTop.innerHTML = MsIeCrFix ( oResponses[i+1] );

                        // these have to be after the setting of innerHTML; otherwise the checkboxes won't be defined yet
                        if (PerfError) { // if not compiled for perf, only enable LinuxTop
                            SetCheckboxDisabled ( "checkboxLinuxTop", false );
                            SetCheckboxDisabled ( "checkboxPerfDeep", true );
                            SetCheckboxDisabled ( "checkboxPerfTop", true );
                        } else {
                            SetCheckboxDisabled ( "checkboxLinuxTop", true );
                            SetCheckboxDisabled ( "checkboxPerfDeep", true );
                            SetCheckboxDisabled ( "checkboxPerfTop", false );
                        }
                    } else if (entry == "PERFDEEPRESULTS" && GetCheckboxStatus("checkboxPerfDeep")) {
                        objPerfTop.innerHTML = MsIeCrFix ( oResponses[i+1] );

                        // these have to be after the setting of innerHTML; otherwise the checkboxes won't be defined yet
                        SetCheckboxDisabled ( "checkboxPerfTop", true );
                        SetCheckboxDisabled ( "checkboxLinuxTop", true );
                    }
                    //alert(entry +": adding to PERFUTILS TH html (" + objPerfTop.innerHTML + ")" );
                }

                // we just re-filled the navigation checkboxes; update the one for context switches
                SetCheckboxStatus ( "checkboxContextSwitch", GetContextSwitch );

                var objTextareaTopResults = document.getElementById("textareaTopResults");
                if (objTextareaTopResults) {
                    objTextareaTopResults.rows = Number(objTextareaTopResults.scrollHeight/14,0) + 1;
                    //alert(entry + ": objTextareaTopResults rows:" + objTextareaTopResults.rows );
                }

                if (entry == "PerfTop") {
                    var checkbox = document.getElementById('checkboxPerfTop');
                    if (checkbox ) {
                        checkbox.checked = GetPerfTop.Value;
                        SetCheckboxDisabled ( "checkboxPerfTop", false );
                        SetCheckboxDisabled ( "checkboxLinuxTop", true );
                    }
                } else if (entry == "LinuxTop") {
                    var checkbox = document.getElementById('checkboxLinuxTop');
                    if (checkbox ) { checkbox.checked = GetLinuxTop.Value; }
                } else if (entry == "PERFDEEPRESULTS") {
                    var checkbox = document.getElementById('checkboxPerfDeep');
                    if (checkbox ) { checkbox.checked = GetPerfDeep.Value; }
                    objTextareaTopResults = document.getElementById("textareaPerfDeep");
                    if (objTextareaTopResults) {
                        objTextareaTopResults.rows = Number(objTextareaTopResults.scrollHeight/14,0) + 1;
                        //alert("TEXTAREA height (" + objTextareaTopResults.scrollHeight + "); numrows (" + objTextareaTopResults.rows + ")" );
                    }
                }

                if (oResponses[i+1].length > 0) {
                    validResponseReceived = 1;
                    //hideOrShow("row_profiling", SHOW );
                }
            } else {
                alert ("element PERFUTILS not found");
            }
            //AddToDebugOutput ( entry + ": len3:" + oResponses[i+1].length + "; " + oResponses[i+1] + eol );

            if (GetPerfDeep.Duration) SetInputValue ( "PerfDeepDuration", GetPerfDeep.Duration );
            if (GetPerfCache.Duration) SetInputValue ( "PerfCacheDuration", GetPerfCache.Duration );
            i++;
        } else if (entry == "CONTEXTSWITCH") {
            AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
            var checkbox = document.getElementById('checkboxContextSwitch');
            if ( checkbox && checkbox.checked ) {
                var objspan = document.getElementById('spanContextSwitches');
                if (objspan) {
                    objspan.innerHTML = "&nbsp;(" + oResponses[i+1] + ")";
                } else {
                    alert ( entry + ": object spanContextSwitches not found");
                }
            } else {
                //alert ( entry + ": box not checked");
            }
            i++;
        } else if (entry == "PerfFlameInit") {
            //AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
            var checkbox = document.getElementById('checkboxPerfFlame');
            if ( checkbox && checkbox.checked ) {
                var objspan = document.getElementById('PerfFlameContents');
                if (objspan) {
                    objspan.innerHTML = oResponses[i+1];
                } else {
                    alert ( entry + ": object PerfFlameContents not found");
                }
                var objtemp = document.getElementById('PerfFlameDuration');
                if ( objtemp ) {
                    objtemp.innerHTML = "";
                }
            } else {
                //alert ( entry + ": box not checked");
            }
            i++;
        } else if (entry == "PERFFLAMESTATUS") {
            AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
            if (GetPerfFlame.State > GetPerfFlameState.IDLE) {
                var objtemp = document.getElementById('PerfFlameSize');
                if ( objtemp ) {
                    var sizeBytes = oResponses[i+1]; // update the size of the perf.data file
                    if ( sizeBytes > 1024*1024) { // megabytes
                        var megabytes = sizeBytes / 1024 / 1024;
                        objtemp.innerHTML = megabytes.toFixed(1) + "MB";
                    } else if ( sizeBytes > 1024) { // kilobytes
                        var kilobytes = sizeBytes / 1024;
                        objtemp.innerHTML = kilobytes.toFixed(1) + "KB";
                    } else {
                        objtemp.innerHTML = sizeBytes;
                    }
                }
                var epochSeconds = Math.floor(localtime.getTime() / 1000) - GetPerfFlameRecordingSeconds;
                objtemp = document.getElementById('PerfFlameDuration');
                if ( objtemp ) {
                    var minutes = Math.floor( epochSeconds / 60 );
                    var seconds = epochSeconds - (minutes * 60 );
                    if (minutes > 0) {
                        objtemp.innerHTML = minutes + "m" + seconds + "s";
                    } else {
                        objtemp.innerHTML = epochSeconds + "s";
                    }
                }
                if (GetPerfFlame.State == GetPerfFlameState.STOP) { // oce the record has stopped, proceed to next state ... CREATESCRIPTOUT
                    GetPerfFlameSetState( GetPerfFlameState.CREATESCRIPTOUT);
                }
            }
            i++;
        } else if (entry == "PERFFLAMEPIDCOUNT") {
            var pidCountNow = oResponses[i+1];
            AddToDebugOutput ( entry + ": pidCountNow:" + pidCountNow + "; GetPerfFlamePidCount:" + GetPerfFlamePidCount + "; State:" + GetPerfFlame.State + eol );
            //alert("PidCount:" + GetPerfFlamePidCount + "; Now:" + pidCountNow );
            if (GetPerfFlame.State == GetPerfFlameState.RECORDING) {
                // if the server reported that the 'perf record' pids are no longer active, something caused the record to end
                if ( pidCountNow == 0 && GetPerfFlamePidCount > 0 ) {
                    //alert("ALERT ... looks like perf record is over"); // either the app it was monitoring exited or something unexpected caused it to exit prematurely
                    GetPerfFlameSetStop();
                }
            }
            GetPerfFlamePidCount = pidCountNow;
            i++;
        } else if (entry == "PERFRECORDUUID") {
            PerfRecordUuid = oResponses[i+1];
            //alert( entry + ":" + PerfRecordUuid );
        } else if (entry == "PERFSCRIPTDONE") {
            //alert( entry + ":" + oResponses[i+1] );
            GetPerfFlameSetState( GetPerfFlameState.GETSVG);
            GetSvgContents();
            i++;
        } else if (entry == "PerfFlameSvgContents") {
            var svglength = oResponses[i+1].length;
            var objtemp = document.getElementById('PerfFlameSvg');
            if ( objtemp ) {
                objtemp.innerHTML = oResponses[i+1];
                //alert("svg:" + objtemp.innerHTML.substr(0,300) );
            }
            i++;
            GetPerfFlameSetState( GetPerfFlameState.IDLE );
        } else if (entry == "PERFFLAME_DELETEOUTFILE_DONE") {
            //alert(entry + ":" + oResponses[i+1] );
            GetPerfFlameSetState( GetPerfFlameState.IDLE );
            i++;
        } else if ( entry == "DvfsControl" ) {
            //AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
            var checkboxDvfsControl = document.getElementById('checkboxDvfsControl');
            if ( checkboxDvfsControl && checkboxDvfsControl.checked ) {
                var DvfsControl = document.getElementById('DvfsControl');
                if ( DvfsControl ) {
                    DvfsControl.innerHTML = oResponses[i+1];
                }
            }
            i++;
        } else if ( entry == "GovernorSetting" ) {
            AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
            var checkboxDvfsControl = document.getElementById('checkboxDvfsControl');
            if ( checkboxDvfsControl && checkboxDvfsControl.checked ) {
                var objGovernorSetting = document.getElementById('radioGovernor' + oResponses[i+1] );
                if ( objGovernorSetting ) {
                    objGovernorSetting.checked = true;
                }
            }
            i++;
        } else {
            if (entry.length > 1 ) {
                AddToDebugOutput ( entry + eol );
            }
        }
    } // end for each response
    if(debug) console.log( "------------------------------------------------" + eol );
}

function clearTimeoutOneSecond( caller ) {
    // if previous timeout is already pending, cancel it
    if (CgiTimeoutId) {
        //console.log( caller + ": calling clearTimeout()");
        clearTimeout(CgiTimeoutId);
        CgiTimeoutId = 0;
    }
}

function clearTimeoutFiveSecond()
{
	if (SpliceTimeoutId){
		clearTimeout(SpliceTimeoutId);
		SpliceTimeoutId = 0;
	}
}

// This function runs as an asynchronous response to a previous server request
function serverHttpResponse ()
{
    var debug=GlobalDebug;
    var idx=0;
    var idx2=0;

    if (debug) alert("serverHttpResponse: got readyState " + xmlhttp.readyState );

    if (xmlhttp.readyState==4 ) {
        //alert("serverHttpResponse: got readyState " + xmlhttp.readyState + "; status " + xmlhttp.status );

        // only if "OK"
        if (xmlhttp.status == 200) {

            debug=0;

            if ( CgiRetryTimeoutId ) {
                //console.log( "Response 200: clearTimeout(CgiRetryTimeoutId:" + CgiRetryTimeoutId + ")");
                clearTimeout ( CgiRetryTimeoutId );
                CgiRetryTimeoutId = 0;
            }

            urlSentSuccessfully++; // used to try to determine if a failure is intermittant or we have never been successful
            urlSentRetryCountAfterSuccess = 0;

            localtime = new Date();
            var responseText1 = xmlhttp.responseText.replace(/</g,"&lt;"); // fails on ie7, safari
            var responseText2 = responseText1.replace(/</g,"&lt;"); // fails on ie7, safari

            var responseText = rtrim(xmlhttp.responseText);

            if( RecordControl.Value > 0 ) {
                var seconds = Math.floor(localtime.getTime() / 1000);
                // occasionally, we get two responses within the same second; force two responses to be same time
                if ( PlaybackSTBTIMEsaved != PlaybackSTBTIME ) {
                    RecordFileContents += "\nUNIXTIME " + seconds + "|!|";
                    PlaybackTimeSeconds = seconds;
                    PlaybackSTBTIMEsaved = PlaybackSTBTIME;
                }
                RecordFileSize = RecordFileContents.length;

                var sizeobj=document.getElementById("recordsize");
                if (sizeobj) {
                    var seconds = Math.floor(localtime.getTime() / 1000) - RecordTimeStart;
                    sizeobj.innerHTML =  output_size_in_human( RecordFileSize  ) + " (" + Math.floor(seconds/60) + ":" + ("00" + seconds%60).slice (-2)+ ")"; // leading zero
                }
                var descobj=document.getElementById("recorddescriptor");
                if (descobj) {
                    descobj.innerHTML = "File&nbsp;size:";
                }
            }

            if (debug) console.log ("responseText:len " + responseText.length + "(" + responseText.substr(0,90) + ")" );
            if (debug) console.log ("split");
            var oResponses = responseText.split( "~" );
            if (debug) console.log ("num responses is " + oResponses.length );

            // sometimes the very first response is blank; if this happens, send another request very soon after we receive the blank response
            if (responseText.length == 0) {
                if (debug) console.log ("response is empty; calling setTimeout");
                ResponseCount++;

                clearTimeoutOneSecond( "response 1");
                clearTimeoutFiveSecond();

                //console.log( "Response: calling setTimeout(OneSecond)");
                CgiTimeoutId = setTimeout ('OneSecond()', REFRESH_IN_MILLISECONDS/10 );
                SpliceimeoutId = setTimeout ('FiveSecond()', REFRESH_IN_MILLISECONDS/2 );
                AddToDebugOutput ( "calling setTimeout(); ID (" + CgiTimeoutId + ")" + eol );
            } else {

                if (objdebug) objdebug.innerHTML = ""; // clear out any previous entries

                //console.log ( "urlSentPreviously: " + urlSentPreviously + eol );
                //console.log ( "urlSentSuccessfully: " + urlSentSuccessfully + "; urlSentRetryCount: " +
                //               urlSentRetryCount + "; urlSentRetryCountAfterSuccess: " + urlSentRetryCountAfterSuccess+ eol );

                ProcessResponses ( oResponses );

                var objwifi = document.getElementById('WIFICOUNTDOWN');
                if (objwifi) {
                    objwifi.innerHTML = "(" + Number(GetWifiStatsCountdown%10) + ")";
                }

                // if this is the response that arrived after a timeout and retry, manually start the one-second timeout
                //console.log( "Response:200 CgiTimeoutId " + CgiTimeoutId );
                if ( CgiTimeoutId == 0 ) {
                    //console.log( "Response:200 calling OneSecond() manually" );
                    OneSecond();
                }
				if ( SpliceTimeoutId == 0) {
					FiveSecond();
				}
            }
        } else {
            var msg = "";

            clearTimeoutOneSecond( "response 2");
            clearTimeoutFiveSecond();

            //console.log ("TIMEOUT1: urlSentSuccessfully:" + urlSentSuccessfully + "; urlSentRetryCount:" + urlSentRetryCount + "; urlSentRetryCountAfterSuccess:" + urlSentRetryCountAfterSuccess );
            // if we have previously successfully received some responses (used so we do not ignore the very first failure)
            if ( ( urlSentSuccessfully > 10) && (urlSentRetryCountAfterSuccess < 5 ) ) { // if we have not had too many retries
                urlSentRetryCount++; // this one should never get reset to zero
                urlSentRetryCountAfterSuccess++; // this one should get reset to zero if we are ever successful in getting a response

                msg = "TIMEOUT2: urlSentSuccessfully:" + urlSentSuccessfully + "; urlSentRetryCount:" + urlSentRetryCount + "; urlSentRetryCountAfterSuccess:" + urlSentRetryCountAfterSuccess;
                //console.log ( msg + eol );

                //console.log ( "calling setTimeout(sendCgiRequestRetry); ID (" + CgiRetryTimeoutId + ")" + eol );
                CgiRetryTimeoutId = setTimeout ('sendCgiRequestRetry()', REFRESH_IN_MILLISECONDS/4 );
            } else {
                msg = "There was a problem retrieving the XML data:" + xmlhttp.statusText;
                //console.log ( msg + eol );
                //console.log ( "urlSendRetryCount is " + urlSentRetryCount + eol );

                if ( urlSentRetryCount < 2 ) {
                    urlSentRetryCount++; // this one should never get reset to zero
                    //console.log ( "urlSentRetryCount is " + urlSentRetryCount + " ... calling setTimeout(sendCgiRequestRetry); ID (" + CgiRetryTimeoutId + ")" + eol );
                    CgiRetryTimeoutId = setTimeout ('sendCgiRequestRetry()', REFRESH_IN_MILLISECONDS/4 );
                }
            }
        }

    } //if (xmlhttp.readyState==4 )
}

function FetchSubmit()
{
    //alert("FetchSubmit returning true");
    return true;
}

function GetRxTxRadioButtonSelected()
{
    return document.getElementById("svg3d_controls").elements["radio_which_svg"].value;
}

function RadioClick(event)
{
    if (GetRxTxRadioButtonSelected() == "RX") {
        sidx=1;
    } else {
        sidx=0;
    }
    //alert("RadioClick: radio button (" + GetRxTxRadioButtonSelected() + "); sidx=" + sidx );
    return true;
}

var WalkResults = "";
var WalkLevel=0;
function WalkAllNodes ( node )
{
    var indentStr = "";
    var visibilityStr = "";
    var displayStr = "";
    var hiddenStr = "";

    if ( ! node ) {
        return true;
    }

    WalkLevel++;
    for(var i = 0; i < WalkLevel*4; i++ ) {
        indentStr += " ";
    }

    if (node.hidden == true ) {
        hiddenStr = ";  hidden";
    }

    if (node.style.visibility == "hidden" ) {
        visibilityStr = ";  visibility=" + node.style.visibility;
    }

    if (node.style.display != "" ) {
        displayStr = ";  display=" + node.style.display;
    }

    WalkResults += indentStr + node.nodeName + ":  id (" + node.id + ") ... nodes " + node.childNodes.length + hiddenStr + visibilityStr + displayStr + eol;
    var children = node.childNodes;

    for(var i = 0; i < node.childNodes.length; i++ ) {
        if ( node.childNodes[i].nodeType == 1) {
            //alert(typeof(node.childNodes[i]) + "- " + node.childNodes[i].nodeType + ": " + node.childNodes[i].tagName + " - " + node.childNodes[i].innerHTML);
            WalkAllNodes ( node.childNodes[i] );
        }
    }
    WalkLevel--;

    return true;
}
function FocusEntryBox(event)
{
    //alert("FocusEntryBox:" + event.target.id );
    iperfEntryBoxUpdate = true;
    return true;
}
function BlurEntryBox(event)
{
    //alert("BlurEntryBox:" + event.target.id );
    iperfEntryBoxUpdate = false;
    return true;
}

// "Client" or "iperfRunningClient"
function set_iperf_cmd( cmd, clientOrServer )
{
    var obj=0;
    if (clientOrServer.indexOf("Client") >= 0 ) {
        obj=document.getElementById('iperf_cmd_c');
    } else {
        obj=document.getElementById('iperf_cmd_s');
    }

    if (obj) {
        obj.innerHTML = cmd;
        //console.log ( "for " + clientOrServer + ": cmd.len " + cmd.length + "; cmd (" + cmd + ")" );
    }
    return true;
}

// event is triggered from onkeyup event ... or could be from "iperf_start_stop_s" or "iperf_start_stop_c"
function KeyupEntryBox(event)
{
    var clientOrServer = "Client";

    //alert("KeyupEntryBox:" + event.target.id );
    var iperfCmd="unknown";
    if ( event == "iperf_start_stop_c") {
        iperfCmd=CreateIperfString( "iperf_start_stop_c" );
    } else if ( event == "iperf_start_stop_s" ) {
        iperfCmd=CreateIperfString( "iperf_start_stop_s" );
        clientOrServer = "Server";
    } else if (event) {
        if ( event.target.id.indexOf("Client") > 0 || event.target.id == "iperf_options_c" || event.target.id == "iperf_addr" || event.target.id == "iperf_duration" || event.target.id == "iperf_port_c" ) {
            iperfCmd=CreateIperfString( "iperf_start_stop_c" );
        } else if ( event.target.id.indexOf("Server") > 0 || event.target.id.indexOf("iperf_options_s") >= 0 ) {
            iperfCmd=CreateIperfString( "iperf_start_stop_s" );
            clientOrServer = "Server";
        }
    }
    set_iperf_cmd(iperfCmd, clientOrServer );
}

function CreateIperfString( clientOrServer )
{
    var local = new Date();
    var iperfCmd = "iperf ";

    if ( clientOrServer == "iperf_start_stop_c" || clientOrServer == "Client" ) {
        if ( document.getElementById('iperf_addr').value.length ) {
            iperfCmd += " -c " + document.getElementById('iperf_addr').value;

            // add a special option to the end of the command line to let other browsers know the start time of this thread
            iperfCmd += " -Q " + Math.floor(local.getTime() / 1000);

            // add the interval count and specify format to be megabits
            iperfCmd += " -i 1 -f m";
        }

        if ( document.getElementById('iperf_duration').value.length ) {
            iperfCmd += " -t " + document.getElementById('iperf_duration').value;
        } else {
            iperfCmd += " -t 100";
        }

        if ( document.getElementById('iperf_port_c').value.length ) {
            iperfCmd += " -p " + document.getElementById('iperf_port_c').value;
        } else {
            iperfCmd += " -p 5001";
        }

        if ( document.getElementById('iperf_options_c').value.length ) {
            iperfCmd += " " + document.getElementById('iperf_options_c').value;
        }
    } else {
        iperfCmd += " -s";

        // add a special option to the end of the command line to let other browsers know the start time of this thread
        iperfCmd += " -Q " + Math.floor(local.getTime() / 1000);

        if ( document.getElementById('iperf_port_s') && document.getElementById('iperf_port_s').value.length ) {
            iperfCmd += " -p" + document.getElementById('iperf_port_s').value + " ";
        } else {
            iperfCmd += " -p 5001";
        }

        if ( document.getElementById('iperf_options_s') && document.getElementById('iperf_options_s').value.length ) {
            iperfCmd += " " + document.getElementById('iperf_options_s').value;
        }
    }

    return iperfCmd;
}
// iperf_start_stop_c, iperfRunningClient ... iperf_start_stop_s, iperfRunningServer
function set_iperf_count_value( clientOrServer, user_seconds )
{
    var local = new Date();
    var seconds = Number( Math.floor(local.getTime() / 1000) );

    // if the caller provided a unix timestamp, use it instead of the current timestamp
    if (user_seconds.length > 5) {
        seconds = user_seconds;
    }
    if (clientOrServer == "iperf_start_stop_c" || clientOrServer == "iperfRunningClient" ) {
        iperfStartTimeClient =  seconds;
    } else {
        iperfStartTimeServer =  seconds;
    }
    return true;
}

function create_duration_string( start_time )
{
    var local = new Date();
    var delta = Math.floor(local.getTime() / 1000) - start_time;
    var deltaStr = "";

    // check for days
    if (delta >= Number(24*60*60) ) {
        deltaStr += Math.floor(delta/Number(24*60*60)) + "d";
        delta = delta - Number(Math.floor(delta/Number(24*60*60))*Number(24*60*60));
    }

    // check for hours
    if ( delta >= 3600) {
        deltaStr += Math.floor(delta/3600) + "h";
        delta = delta - Number(Math.floor(delta/3600)*3600);
    } else {
        if ( deltaStr.length ) {
            deltaStr += "00h";
        }
    }

    if ( deltaStr.length || delta>=60) {
        if (deltaStr.length) { // if hours are present, pad minutes with zero
            deltaStr += Math.floor(delta/60).padZero(2) + "m";
        } else {
            deltaStr += Math.floor(delta/60) + "m";
        }
        delta = delta - Number(Math.floor(delta/60)*60);
    }
    if (deltaStr.length) { // if hours or minutes are present, pad seconds with zero
        deltaStr += delta.padZero(2) + "s";
    } else {
        deltaStr += delta + "s";
    }
    return deltaStr;
}

function set_iperf_count_cell()
{
    var obj=0;
    if ( iperfStateClient != iperfStateEnum.UNINIT ) {
        obj=document.getElementById('iperf_count_c');
        if (obj && iperfStartTimeClient ) {
            obj.innerHTML = create_duration_string( iperfStartTimeClient );
        }
    }

    if ( iperfStateServer != iperfStateEnum.UNINIT ) {
        obj=document.getElementById('iperf_count_s');
        if (obj && iperfStartTimeServer ) {
            obj.innerHTML = create_duration_string( iperfStartTimeServer );
        }
    }
    return true;
}

function set_iperf_status_cell()
{
    var obj=document.getElementById('iperf_status_c');
    if (obj ) {
        if ( iperfStateClient == iperfStateEnum.UNINIT ) {
            obj.innerHTML = "IDLE";
            iperfClientRate = "";
        } else if ( iperfStateClient == iperfStateEnum.INIT ) {
            obj.innerHTML = "INIT";
        } else if ( iperfStateClient == iperfStateEnum.RUNNING ) {
            obj.innerHTML = "RUNNING";
        } else if ( iperfStateClient == iperfStateEnum.STOP ) {
            obj.innerHTML = "STOP";
        } else {
            obj.innerHTML = "UKNOWN";
        }

        if (iperfPidClient.length > 8) { // terminated or Executable not found
            obj.innerHTML += " ... " + iperfPidClient;
        } else if (iperfPidClient.length > 1) { // 5-digit pid
            obj.innerHTML += " ... PID " + iperfPidClient;
            if ( iperfClientRate.length > 0 ) obj.innerHTML += " (" + iperfClientRate + " Mbps)";
        }
    }

    obj=document.getElementById('iperf_status_s');
    if (obj ) {
        if ( iperfStateServer == iperfStateEnum.UNINIT ) {
            obj.innerHTML = "IDLE";
        } else if ( iperfStateServer == iperfStateEnum.INIT ) {
            obj.innerHTML = "INIT";
        } else if ( iperfStateServer == iperfStateEnum.RUNNING ) {
            obj.innerHTML = "RUNNING";
        } else if ( iperfStateServer == iperfStateEnum.STOP ) {
            obj.innerHTML = "STOP";
        } else {
            obj.innerHTML = "UKNOWN";
        }

        if (iperfPidServer.length > 8) { // terminated or Executable not found
            obj.innerHTML += " ... " + iperfPidServer;
        } else if (iperfPidServer.length > 1) { // 5-digit pid
            obj.innerHTML += " ... PID " + iperfPidServer;
        }
    }
    return true;
}

function set_iperf_stop()
{
    if ( iperfStateClient == iperfStateEnum.RUNNING ) {
        iperfStateClient = iperfStateEnum.STOP;
    }
    iperfTimeoutClient = 0;
    iperfPidClient = "";

    return true;
}
// "iperfPidClient", "iperfErrorClient"
function set_iperf_button( clientOrServer )
{
    var obj=0;
    var state=0;
    if (clientOrServer == "iperf_start_stop_c" || clientOrServer.indexOf("Client") >= 0 ) {
        obj=document.getElementById('iperf_start_stop_c');
        state = iperfStateClient;
    } else {
        obj=document.getElementById('iperf_start_stop_s');
        state = iperfStateServer;
    }
    if (obj) {
        if ( state == iperfStateEnum.INIT ) {
            obj.value = "STOP";
        } else  if ( state == iperfStateEnum.STOP || state == iperfStateEnum.UNINIT ) {
            obj.value = "START";
        }
    }

    return true;
}

function OnLoadEvent222(evt)
{
  //alert("OnLoadEvent222: " + evt.target.id );
  parent.AddSVGObject(evt.target.ownerDocument, evt.target.id );
}

function OnLoadEvent333(evt)
{
  //alert("OnLoadEvent333: " + evt.target.id );
  parent.AddSVGObject(evt.target.ownerDocument, evt.target.id );
}

function fillin_iperf_entry_boxes()
{
    //alert(iperfRunningClient.substr(iperfRunningClient.indexOf("iperf")));
    var tokens = iperfRunningClient.substr(iperfRunningClient.indexOf("iperf")).split(" ");
    var obj=0;
    var extra = "";
    for(var i=0; i<tokens.length; i++) {
        var value = "";
        if (tokens[i] > 2) {
            value = tokens[i].substr(2);
        } else {
            value = tokens[i+1];
        }
        if (tokens[i] == "iperf") {
        } else if (tokens[i] == "-t") {
            SetInputValue ( "iperf_duration", value );
            i++;
        } else if (tokens[i] == "-c") {
            SetInputValue ( "iperf_addr", value );
            i++;
        } else if (tokens[i] == "-p") {
            SetInputValue ( "iperf_port", value );
            i++;
        } else {
            if ( extra.length ) {
                extra += " ";
            }
            extra += tokens[i];
        }
    }
    if ( extra.length ) {
        SetInputValue ( "iperf_options_c", extra );
    }
}

// this function will try to return the unix seconds found in the iperf command line.
function get_unix_seconds( mystring )
{
    var seconds = 0;
    var offset=mystring.indexOf(' -Q');
    if ( offset > 0) {
        var partial=rtrim(mystring.substr(Number(offset+3)));
        var pieces=partial.split(' ');
        if (pieces.length==1) {
            seconds = pieces[0];
        } else if (pieces.length>1) {
            seconds = pieces[1];
        }
    } else {
        var local = new Date();
        seconds = Math.floor(local.getTime() / 1000);
    }

    return seconds;
}

/*
function FetchSubmit()
{
    sRecordFilename = "bsysperf_record_" + UUID + ".txt";
    var obj=document.getElementById("recordfile");
    console.log("FetchSubmit: file (" + sRecordFilename + "); obj " + obj);
    if (obj) {
        obj.value = sRecordFilename;
        obj.innerHTML = sRecordFilename;
        return true;
    }

    return false;
}

function StateFileSelected(event)
{
    console.log("stateFileSelected");
    document.forms['formdownload'].submit();
    return false;
}

function validateForm(event)
{
    var id=event.target.id;
    var objfile = document.getElementById('btnstatefile');
    console.log("validate form: id " + id + "; sCaptureFilename (" + sCaptureFilename + "); selectedFile (" + objfile.value + ")" );
    if (sCaptureFilename.length == 0) {
        console.log("file len " + objfile.value.length );
        alert("File name is empty; browse for a file.");
        return false;
    }
    return false;
}
*/

function makeTextFile (text)
{
    var data = new Blob([text], {type: 'text/plain'});

    // If we are replacing a previously generated file we need to
    // manually revoke the object URL to avoid memory leaks.
    if ( gTextFile !== null) {
        window.URL.revokeObjectURL( gTextFile );
    }

    var gTextFile = window.URL.createObjectURL(data);

    // returns a URL you can use as a href
    return gTextFile;
}

/**
 *  Function: This function will handle the processing when the user selects the "Save File" button.
 **/
function RecordSaveFileListener ()
{
    var link = document.createElement('a');

    //console.log("calling RecordPlaybackConfigure (START) ");

    link.setAttribute('download', 'bsysperf_record_' + UUID + '.txt' );
    link.href = makeTextFile( RecordFileContents );
    document.body.appendChild(link);

    // wait for the link to be added to the document
    window.requestAnimationFrame(function () {
        var event = new MouseEvent('click');
        link.dispatchEvent(event);
        document.body.removeChild(link);
    }); // end call to requestAnimationFrame
}

/**
 *  Function: This function will handle the processing when the user selects the "Export CPU Stats" button.
 **/
function ExportCpuStatsListener ()
{
    var link = document.createElement('a');
    var csv = "";

    link.setAttribute('download', 'bsysperf_cpustats_' + UUID + '.csv' );

    // loop through each array element and extract the CPU utilization numbers
    for ( var idx=0; idx<RecordFileContentsArray.length ; idx++ ) {
        // look for CPUINFO tag
        if ( RecordFileContentsArray[idx] ) {
            // "1479847989|!|~CPUINFO~4 000098 000097 000097 000097 ~NETBYTES~8956436 13474876,53252774358 53252774358,94926 672,~
            var temp1 = RecordFileContentsArray[idx];
            var temp2 = RecordFileContentsArray[idx].indexOf("CPUINFO");
            if ( RecordFileContentsArray[idx].indexOf("CPUINFO") > 0 ) /* only interested in lines that have CPUINFO in them */ {
                var time_elements = RecordFileContentsArray[idx].split("|!|");  // this should give us two elements
                if ( time_elements.length > 1 ) {
                    var elements = time_elements[1].split("~CPUINFO~");  // this should give us two elements
                    if ( elements.length > 1) {
                        var CPUINFO = elements[1].split("~"); // this should give us 4 000096 000099 000023 000045
                        if ( csv.length > 0 ) csv += "\n";
                        var CPUINFO_trimmed = rtrim(CPUINFO[0]);
                        var num_cpus = CPUINFO_trimmed.substr(0,1);

                        // if this is the first line in the file, output the header information
                        if ( csv.length == 0 ) {
                            csv += "UNIXTIME";
                            for ( var cpu=0; cpu<num_cpus; cpu++ ) {
                                csv += ",CPU " + cpu;
                            }
                            csv += "\n";
                        }
                        var with_commas = CPUINFO_trimmed.substr(2,99).replace(/ /g, ","); // replace every space with a comma
                        csv += time_elements[0] + "," + with_commas;
                    }
                }
            }
        }
    }
    link.href = makeTextFile( csv );
    document.body.appendChild(link);

    // wait for the link to be added to the document
    window.requestAnimationFrame(function () {
        var event = new MouseEvent('click');
        link.dispatchEvent(event);
        document.body.removeChild(link);
    }); // end call to requestAnimationFrame
}

/**
 *  Function: This function will handle the processing when the user selects the "Export Net Stats" button.
 **/
function ExportNetStatsListener ()
{
    var link = document.createElement('a');
    var csv = "";

    link.setAttribute('download', 'bsysperf_netstats_' + UUID + '.csv' );

    // loop through each array element and extract the CPU utilization numbers
    for ( var idx=0; idx<RecordFileContentsArray.length ; idx++ ) {
        // look for CPUINFO tag
        if ( RecordFileContentsArray[idx] ) {
            // "1479847989|!|~CPUINFO~4 000098 000097 000097 000097 ~NETBYTES~8956436 13474876,53252774358 53252774358,94926 672,~
            var temp1 = RecordFileContentsArray[idx];
            var temp2 = RecordFileContentsArray[idx].indexOf("NETBYTES");
            if ( RecordFileContentsArray[idx].indexOf("NETBYTES") > 0 ) /* only interested in lines that have NETBYTES in them */ {
                var time_elements = RecordFileContentsArray[idx].split("|!|");  // this should give us two elements
                if ( time_elements.length > 1 ) {
                    var elements = time_elements[1].split("~NETBYTES~");  // this should give us two elements
                    if ( elements.length > 1) {
                        var NETBYTES = elements[1].split("~"); // this should give us 8956436 13474876,53252774358 53252774358,94926 672,
                        if ( csv.length > 0 ) csv += "\n";
                        var NETBYTES_trimmed = rtrim(NETBYTES[0]);

                        if ( NETBYTES_trimmed.length ) {
                            var data = "";
                            if ( NETBYTES_trimmed[NETBYTES_trimmed.length - 1] == ',' ) {
                                data = NETBYTES_trimmed.substr(0, NETBYTES_trimmed.length -2); // do not include the end-of-line comma
                            } else {
                                data = NETBYTES_trimmed;
                            }

                            var eth_entries = data.split(","); // should give us 3 entries

                            // if this is the first line in the file, output the header information
                            if ( csv.length == 0 ) {
                                csv += "UNIXTIME";
                                var obj = 0;
                                for ( var eth=0; eth<eth_entries.length; eth++ ) {
                                    obj = document.getElementById ( "ethname" + eth ); //look for the ethernet interface name
                                    if ( obj ) {
                                        csv += "," + obj.innerHTML + " RX," + obj.innerHTML + " TX"; // we found the interface name
                                    } else {
                                        csv += ",ETH " + eth + " RX,ETH " + eth + " TX"; // could not find interface name
                                    }
                                }
                                csv += "\n";
                            }
                            csv += time_elements[0];
                            for (eth=0; eth<eth_entries.length; eth++) {
                                var rxtx = eth_entries[eth].split(" "); // this will give us the RX and TX entries for this eth interface
                                csv += "," + rxtx[0] + "," + rxtx[1];
                            }
                            csv += "\n";
                        }
                    }
                }
            }
        }
    }

    link.href = makeTextFile( csv );
    document.body.appendChild(link);

    // wait for the link to be added to the document
    window.requestAnimationFrame(function () {
        var event = new MouseEvent('click');
        link.dispatchEvent(event);
        document.body.removeChild(link);
    }); // end call to requestAnimationFrame
}

/**
 *  Function: This function will clear out the polylines that are used to the CPU Utilization and
 *            also those used for network utilization.
 **/
function clearOutCpuGraphsAndTextareas()
{
    // polylines 1-4 are for CPUs 1-4; 0 is the red one for average; gNumCpus one is for green 5-sec average
    for (var polylinenumber=0; polylinenumber<(gNumCpus+1) ; polylinenumber++) { //once for all CPUs
        var lineid = "polyline0" + polylinenumber;
        var svgobj = document.getElementById(lineid);
        if (svgobj) {
            svgobj.setAttribute('points', "20,100 " );
        }
        var textareaobj = document.getElementById( "cpudata0" + Number(polylinenumber-1) ); // textareas for cpu start with zero
        if (textareaobj ) {
            textareaobj.innerHTML = "";
        }
    }


    // repeat process for possible 10 network graphics polylines
    for ( var netIfIdx=0; netIfIdx<10; netIfIdx++) {
        ClearNetGfxGraph ( netIfIdx);
    }
}

/**
 *  Function: This function will find all live elements that are enabled when the user
 *            starts a playback. When Playback begins, we want to disable user inputs
 *            so that the Playback is not confused with live user inputs.
 **/
function FindInputElementsThatAreEnabled ()
{
    var tag;
    var inputs = document.getElementsByTagName("input");

    // you can also use var allElem=document.all; and loop on it
    for(i = 0; i < inputs.length; i++) {
        //tag = inputs[i].tagName;
        //console.log ( "document.all(" + i + ")" );
        if ( inputs[i] && inputs[i].id && inputs[i].type ) {
            //console.log ( "type-> " + inputs[i].type + " ... id-> " + inputs[i].id );
            tag = "(" + inputs[i].type + ":" + inputs[i].id + ")";
            if ( inputs[i].type == "checkbox" || inputs[i].type == "button" || inputs[i].type == "submit" ) {
                var obj= document.getElementById( inputs[i].id );
                if ( obj && ( obj.disabled == false ) ) {
                    //console.log ( tag + " is enabled" );

                    // it is possible user added new input elements since the last pass through (checked Wifi box)
                    if ( PlaybackElementsDisabled.indexOf ( inputs[i].id ) !== -1 ) { // element already in list
                        // do not add the same element again
                        //console.log( "FindInputElementsThatAreEnabled: skipping ... " + inputs[i].id );
                    } else {
                        if (PlaybackElementsDisabled.length > 0 ) PlaybackElementsDisabled += ",";
                        PlaybackElementsDisabled += inputs[i].id;
                    }
                    setButtonDisabled ( inputs[i].id, true );
                } else if ( obj && ( obj.disabled == true ) ) {
                    //console.log ( tag + " is already disabled" );
                }
            }
        }
    }

    //console.log ("PlaybackElementsDisabled: (" + PlaybackElementsDisabled + ")" );
}

/**
 *  Function: This function will handle the processing when the user selects a new Playback file.
 **/
function readText(that)
{
    //console.log("readText( top of function)");
    if(that.files && that.files[0]) {
        RecordPlaybackConfigure ( "START" );

        var reader = new FileReader();
        reader.onload = function (e) {
            RecordFileContents2 = e.target.result;
            //alert("length = " + RecordFileContents2.length + "; contents is (" + RecordFileContents2 + ")" );
            RecordFileContentsArray = RecordFileContents2.split("UNIXTIME ");

            clearOutCpuGraphsAndTextareas();

            clearTimeoutOneSecond( "readText" );

            PlaybackFileContentsArrayIdx = 1; // the 0th entry is always empty; skip it and start with 1st entry

            PlaybackControl.Value = 1;

            setButtonImage ( "buttonPlaybackRun", "bmemperf_pause.jpg" );

            PlaybackElementsDisabled = "";

            FindInputElementsThatAreEnabled ();

            // unlike the other inputs, these two buttons should get enabled but never disabled
            setButtonDisabled( 'buttonExportCpuStats', false );
            setButtonDisabled( 'buttonExportNetStats', false );

            UpdateProgressGraph ();

            ProcessPlaybackEvents();
        };//end onload()
        var filename = that.files[0];
        reader.readAsText(that.files[0]);
    }//end if html5 filelist support
}

/**
 *  Function: This function will output a human-readable number of bytes.
 **/
function output_size_in_human( num_bytes )
{
    if ( num_bytes < 1024)
    {
        return ( num_bytes + " bytes" );
    }
    else if ( num_bytes < 1024*1024)
    {
        return ( Number( num_bytes / 1024).toFixed(1) + " KB" );
    }
    else if ( num_bytes < 1024*1024*1024)
    {
        return ( Number( num_bytes / 1024 / 1024).toFixed(1) + " MB" );
    }
    else
    {
        return ( Number( num_bytes / 1024 / 1024 / 1024).toFixed(1) + " GB" );
    }
}
