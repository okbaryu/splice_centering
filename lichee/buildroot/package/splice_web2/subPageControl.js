function changePage(tag){
  var server_addr = getCookie('server_addr');
  //belt.php
  var _sCenteringParam = document.getElementById('sCenteringParam');
  var _sActuator = document.getElementById('sActuator');
  var _sCamera = document.getElementById('sCamera');
  var _sEncoder = document.getElementById('sEncoder');
  var _sSystemOverview = document.getElementById('sSystemOverview');
  var _sLogData = document.getElementById('sLogData');
  var _eBlock1 = document.getElementById('eBlock1');
  var _eBlock2 = document.getElementById('eBlock2');
  var _eBlock3 = document.getElementById('eBlock3');
  var _eBlock4 = document.getElementById('eBlock4');
  var _eBlock5 = document.getElementById('eBlock5');
  var _eBlock6 = document.getElementById('eBlock6');

  //centeringParam.php
  var _sSpec = document.getElementById('sSpec');
  var _sStandard = document.getElementById('sStandard');
  var _s7Offset = document.getElementById('s7Offset');
  var _sAutoSplice = document.getElementById('sAutoSplice');

  //camera.php
  var _sCalibration = document.getElementById('sCalibration');
  var _sCameraStatus = document.getElementById('sCameraStatus');

  //plcSetting.php
  var _sConnection = document.getElementById('sConnection');
  var _sDataMonitor = document.getElementById('sDataMonitor');

  //webServerSetting.php
  var _sSetting = document.getElementById('sSetting')

  var container = document.getElementById('subPage');

  switch (tag) {
    //belt.php
    case 'sCenteringParam':
      _sCenteringParam.style.backgroundColor = '#1D1E21';
      _sActuator.style.backgroundColor = '#45464d';
      _sCamera.style.backgroundColor = '#45464d';
      _sEncoder.style.backgroundColor = '#45464d';
      _sSystemOverview.style.backgroundColor = '#45464d';
      _sLogData.style.backgroundColor = '#45464d';
      _eBlock1.style.backgroundColor = '#1D1E21';
      _eBlock2.style.backgroundColor = 'transparent';
      _eBlock3.style.backgroundColor = 'transparent';
      _eBlock4.style.backgroundColor = 'transparent';
      _eBlock5.style.backgroundColor = 'transparent';
      _eBlock6.style.backgroundColor = 'transparent';
      container.innerHTML = `<iframe id="subP1" src="http://${server_addr}/cgi/php/home/applicationSetting/systemSetting/belt/centeringParam.php" frameborder="0" scrolling="no" width="730" height="600"></iframe>`;
      localStorage.iFrameCnt = 1;
      break;
    case 'sActuator':
      _sCenteringParam.style.backgroundColor = '#45464d';
      _sActuator.style.backgroundColor = '#1D1E21';
      _sCamera.style.backgroundColor = '#45464d';
      _sEncoder.style.backgroundColor = '#45464d';
      _sSystemOverview.style.backgroundColor = '#45464d';
      _sLogData.style.backgroundColor = '#45464d';
      _eBlock1.style.backgroundColor = 'transparent';
      _eBlock2.style.backgroundColor = '#1D1E21';
      _eBlock3.style.backgroundColor = 'transparent';
      _eBlock4.style.backgroundColor = 'transparent';
      _eBlock5.style.backgroundColor = 'transparent';
      _eBlock6.style.backgroundColor = 'transparent';
      container.innerHTML = `<iframe id="subP1" src="http://${server_addr}/cgi/php/home/applicationSetting/systemSetting/belt/actuator.php" frameborder="0" scrolling="no" width="745" height="619"></iframe>`;
      localStorage.iFrameCnt = 1;
      break;
    case 'sCamera':
      _sCenteringParam.style.backgroundColor = '#45464d';
      _sActuator.style.backgroundColor = '#45464d';
      _sCamera.style.backgroundColor = '#1D1E21';
      _sEncoder.style.backgroundColor = '#45464d';
      _sSystemOverview.style.backgroundColor = '#45464d';
      _sLogData.style.backgroundColor = '#45464d';
      _sLogData.style.backgroundColor = '#45464d';
      _eBlock1.style.backgroundColor = 'transparent';
      _eBlock2.style.backgroundColor = 'transparent';
      _eBlock3.style.backgroundColor = '#1D1E21';
      _eBlock4.style.backgroundColor = 'transparent';
      _eBlock5.style.backgroundColor = 'transparent';
      _eBlock6.style.backgroundColor = 'transparent';
      container.innerHTML = `<iframe id="subP1" src="http://${server_addr}/cgi/php/home/applicationSetting/systemSetting/belt/camera.php" frameborder="0" scrolling="no" width="730" height="600"></iframe>`;
      localStorage.iFrameCnt = 1;
      break;
    case 'sEncoder':
      _sCenteringParam.style.backgroundColor = '#45464d';
      _sActuator.style.backgroundColor = '#45464d';
      _sCamera.style.backgroundColor = '#45464d';
      _sEncoder.style.backgroundColor = '#1D1E21';
      _sSystemOverview.style.backgroundColor = '#45464d';
      _sLogData.style.backgroundColor = '#45464d';
      _sLogData.style.backgroundColor = '#45464d';
      _eBlock1.style.backgroundColor = 'transparent';
      _eBlock2.style.backgroundColor = 'transparent';
      _eBlock3.style.backgroundColor = 'transparent';
      _eBlock4.style.backgroundColor = '#1D1E21';
      _eBlock5.style.backgroundColor = 'transparent';
      _eBlock6.style.backgroundColor = 'transparent';
      container.innerHTML = `<iframe id="subP1" src="http://${server_addr}/cgi/php/home/applicationSetting/systemSetting/belt/encoder.php" frameborder="0" scrolling="no" width="745" height="619"></iframe>`;
      localStorage.iFrameCnt = 1;
      break;
    case 'sSystemOverview':
      _sCenteringParam.style.backgroundColor = '#45464d';
      _sActuator.style.backgroundColor = '#45464d';
      _sCamera.style.backgroundColor = '#45464d';
      _sEncoder.style.backgroundColor = '#45464d';
      _sSystemOverview.style.backgroundColor = '#1D1E21';
      _sLogData.style.backgroundColor = '#45464d';
      _sLogData.style.backgroundColor = '#45464d';
      _eBlock1.style.backgroundColor = 'transparent';
      _eBlock2.style.backgroundColor = 'transparent';
      _eBlock3.style.backgroundColor = 'transparent';
      _eBlock4.style.backgroundColor = 'transparent';
      _eBlock5.style.backgroundColor = '#1D1E21';
      _eBlock6.style.backgroundColor = 'transparent';
      container.innerHTML = `<iframe id="subP1" src="http://${server_addr}/cgi/php/home/applicationSetting/systemSetting/belt/systemOverview.php" frameborder="0" scrolling="no" width="745" height="619"></iframe>`;
      localStorage.iFrameCnt = 1;
      break;
    case 'sLogData':
      _sCenteringParam.style.backgroundColor = '#45464d';
      _sActuator.style.backgroundColor = '#45464d';
      _sCamera.style.backgroundColor = '#45464d';
      _sEncoder.style.backgroundColor = '#45464d';
      _sSystemOverview.style.backgroundColor = '#45464d';
      _sLogData.style.backgroundColor = '#1D1E21';
      _eBlock1.style.backgroundColor = 'transparent';
      _eBlock2.style.backgroundColor = 'transparent';
      _eBlock3.style.backgroundColor = 'transparent';
      _eBlock4.style.backgroundColor = 'transparent';
      _eBlock5.style.backgroundColor = 'transparent';
      _eBlock6.style.backgroundColor = '#1D1E21';
      container.innerHTML = `<iframe id="subP1" src="http://${server_addr}/cgi/php/home/applicationSetting/systemSetting/belt/logData.php" frameborder="0" scrolling="no" width="745" height="619"></iframe>`;
      localStorage.iFrameCnt = 1;
      break;

    //centeringParam.php
    case 'sSpec':
      _sSpec.style.backgroundColor = '#D63E2E';
      _sStandard.style.backgroundColor = '#5E5F67';
      _s7Offset.style.backgroundColor = '#5E5F67';
      _sAutoSplice.style.backgroundColor = '#5E5F67';
      container.innerHTML = '<iframe src="./centeringParam/spec.php" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 2;
      break;
    case 'sStandard':
      _sSpec.style.backgroundColor = '#5E5F67';
      _sStandard.style.backgroundColor = '#D63E2E';
      _s7Offset.style.backgroundColor = '#5E5F67';
      _sAutoSplice.style.backgroundColor = '#5E5F67';
      container.innerHTML = '<iframe src="./centeringParam/standard.php" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 2;
      break;
    case 's7Offset':
      _sSpec.style.backgroundColor = '#5E5F67';
      _sStandard.style.backgroundColor = '#5E5F67';
      _s7Offset.style.backgroundColor = '#D63E2E';
      _sAutoSplice.style.backgroundColor = '#5E5F67';
      container.innerHTML = '<iframe src="./centeringParam/7offset.php" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 2;
      break;
    case 'sAutoSplice':
      _sSpec.style.backgroundColor = '#5E5F67';
      _sStandard.style.backgroundColor = '#5E5F67';
      _s7Offset.style.backgroundColor = '#5E5F67';
      _sAutoSplice.style.backgroundColor = '#D63E2E';
      container.innerHTML = '<iframe src="./centeringParam/autoSplice.php" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 2;
      break;

    //camera.php
    case 'sCalibration':
      _sCalibration.style.backgroundColor = '#D63E2E';
      _sCameraStatus.style.backgroundColor = '#5E5F67';
      container.innerHTML = '<iframe src="./camera/cameraCelibration.php" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 2;
      break;
    case 'sCameraStatus':
      _sCalibration.style.backgroundColor = '#5E5F67';
      _sCameraStatus.style.backgroundColor = '#D63E2E';
      container.innerHTML = '<iframe src="./camera/cameraStatus.php" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 2;
      break;

    //webServerSetting.php
    case 'sSetting':
      _sSetting.style.backgroundColor = '#1D1E21';
      _eBlock1.style.backgroundColor = '#1D1E21';
      container.innerHTML = `<iframe id="subP1" src="http://${server_addr}/cgi/php/home/webServerSetting/setting.php" frameborder="0" scrolling="no" width="745" height="619"></iframe>`;
      localStorage.iFrameCnt = 1;
      break;

    //plcSetting.php
    case 'sConnection':
      _sConnection.style.backgroundColor = '#1D1E21';
      _sDataMonitor.style.backgroundColor = '#45464d';
      _eBlock1.style.backgroundColor = '#1D1E21';
      _eBlock2.style.backgroundColor = 'transparent';
      container.innerHTML = '<iframe src="./plcSetting/connection.php" frameborder="0" scrolling="no" width="745" height="619"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
    case 'sDataMonitor':
      _sConnection.style.backgroundColor = '#45464d';
      _sDataMonitor.style.backgroundColor = '#1D1E21';
      _eBlock1.style.backgroundColor = 'transparent';
      _eBlock2.style.backgroundColor = '#1D1E21';
      container.innerHTML = '<iframe src="./plcSetting/dataMonitor.php" frameborder="0" scrolling="no" width="745" height="619"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
  }
}

function historyBack(){
  var url = document.location.href;
  var url_array = url.split("/");
  var dest = "";

  if(url.indexOf("home.php") != -1){
    for(var i = 0; i < 5; i++){
      dest += url_array[i];
      dest += '/';
    }

    dest += "login.php";
  }else{
    for(var i = 0; i < (url_array.length - 2); i++){
      dest += url_array[i];
      dest += '/';
    }

    dest += url_array[url_array.length - 2] + ".php";
  }

  location.href = dest;

  // var cnt;
  // var browser = navigator.userAgent.toLowerCase();
  //
  // if(localStorage.iFrameCnt){
  //   cnt = parseInt(localStorage.iFrameCnt, 10)
  //   cnt = -(1 + cnt);
  //   history.go(cnt);
  //   localStorage.removeItem("iFrameCnt");
  // }else{
  //   if(browser.indexOf("chrome") != -1){
  //     var link = document.location.href;
  //     if(link.indexOf("#") != -1){
  //       history.go(-2);
  //     }else{
  //       history.back();
  //     }
  //   }else{
  //     history.back();
  //   }
  // }
}

function getCookie(c_name){
  var i,x,y,ARRcookies=document.cookie.split(";");

  for (i=0;i<ARRcookies.length;i++)
  {
    x=ARRcookies[i].substr(0,ARRcookies[i].indexOf("="));

    y=ARRcookies[i].substr(ARRcookies[i].indexOf("=")+1);

    x=x.replace(/^\s+|\s+$/g,"");

    if (x==c_name)
    {
        return unescape(y);
    }
  }
}
