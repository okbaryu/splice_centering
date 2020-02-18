function changePage(tag){
  var beltButton = document.getElementById('beltButton');
  var threadButton = document.getElementById('threadButton');
  var carcassButton = document.getElementById('carcassButton');
  var cpsButton = document.getElementById('cpsButton');
  var asButton = document.getElementById('asButton');
  var csButton = document.getElementById('csButton');
  var plcConnectSet = document.getElementById('plcConnectSet');
  var protocolSet = document.getElementById('protocolSet');
  var plcDataLen = document.getElementById('plcDataLen');
  var plcDataMonit = document.getElementById('plcDataMonit');
  var administrator = document.getElementById('administrator');
  var maintenance = document.getElementById('maintenance');
  var operator = document.getElementById('operator');
  var pc = document.getElementById('pc');
  var internalApp = document.getElementById('internalApp');
  var container = document.getElementById('subPage');

  switch (tag) {
    //belt.php
    case 'beltButton':
      beltButton.style.backgroundColor = '#1D1E21';
      threadButton.style.backgroundColor = '#45464d';
      carcassButton.style.backgroundColor = '#45464d';
      container.innerHTML = '<iframe id="subP1" src="./sysSetting/beltSub.php" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
    case 'threadButton':
      beltButton.style.backgroundColor = '#45464d';
      threadButton.style.backgroundColor = '#1D1E21';
      carcassButton.style.backgroundColor = '#45464d';
      container.innerHTML = '<iframe src="./sysSetting/subCon.html" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
    case 'carcassButton':
      beltButton.style.backgroundColor = '#45464d';
      threadButton.style.backgroundColor = '#45464d';
      carcassButton.style.backgroundColor = '#1D1E21';
      container.innerHTML = '<iframe src="./sysSetting/subCon.html" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 1;
      break;

    //beltSub.php
    case 'cpsButton':
      cpsButton.style.backgroundColor = '#D63E2E';
      asButton.style.backgroundColor = '#5E5F67';
      csButton.style.backgroundColor = '#5E5F67';
      container.innerHTML = '<iframe src="./cps.html" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 2;
      break;
    case 'asButton':
      cpsButton.style.backgroundColor = '#5E5F67';
      asButton.style.backgroundColor = '#D63E2E';
      csButton.style.backgroundColor = '#5E5F67';
      container.innerHTML = '<iframe src="./as.html" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 2;
      break;
    case 'csButton':
      cpsButton.style.backgroundColor = '#5E5F67';
      asButton.style.backgroundColor = '#5E5F67';
      csButton.style.backgroundColor = '#D63E2E';
      container.innerHTML = '<iframe src="./cs.html" frameborder="0" scrolling="no" width="730" height="600"></iframe>';
      localStorage.iFrameCnt = 2;
      break;

    //plcSetting.php
    case 'plcConnectSet':
      plcConnectSet.style.backgroundColor = '#1D1E21';
      protocolSet.style.backgroundColor = '#45464d';
      plcDataLen.style.backgroundColor = '#45464d';
      plcDataMonit.style.backgroundColor = '#45464d';
      container.innerHTML = '<iframe src="./plcSetting/plcConSet.html" frameborder="0" scrolling="no" width="745" height="584"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
    case 'protocolSet':
      plcConnectSet.style.backgroundColor = '#45464d';
      protocolSet.style.backgroundColor = '#1D1E21';
      plcDataLen.style.backgroundColor = '#45464d';
      plcDataMonit.style.backgroundColor = '#45464d';
      container.innerHTML = '<iframe src="./plcSetting/protocolSet.html" frameborder="0" scrolling="no" width="745" height="584"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
    case 'plcDataLen':
      plcConnectSet.style.backgroundColor = '#45464d';
      protocolSet.style.backgroundColor = '#45464d';
      plcDataLen.style.backgroundColor = '#1D1E21';
      plcDataMonit.style.backgroundColor = '#45464d';
      container.innerHTML = '<iframe src="./plcSetting/dataLen.html" frameborder="0" scrolling="no" width="745" height="584"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
    case 'plcDataMonit':
      plcConnectSet.style.backgroundColor = '#45464d';
      protocolSet.style.backgroundColor = '#45464d';
      plcDataLen.style.backgroundColor = '#45464d';
      plcDataMonit.style.backgroundColor = '#1D1E21';
      container.innerHTML = '<iframe src="./plcSetting/dataMonitor.html" frameborder="0" scrolling="no" width="745" height="584"></iframe>';
      localStorage.iFrameCnt = 1;
      break;

    //userManagement.php
    case 'administrator':
      administrator.style.backgroundColor = '#1D1E21';
      maintenance.style.backgroundColor = '#45464d';
      operator.style.backgroundColor = '#45464d';
      container.innerHTML = '<iframe src="./sysSetting/subCon1.html" frameborder="0" scrolling="no" width="745" height="584"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
    case 'maintenance':
      administrator.style.backgroundColor = '#45464d';
      maintenance.style.backgroundColor = '#1D1E21';
      operator.style.backgroundColor = '#45464d';
      container.innerHTML = '<iframe src="./sysSetting/subCon1.html" frameborder="0" scrolling="no" width="745" height="584"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
    case 'operator':
      administrator.style.backgroundColor = '#45464d';
      maintenance.style.backgroundColor = '#45464d';
      operator.style.backgroundColor = '#1D1E21';
      container.innerHTML = '<iframe src="./sysSetting/subCon1.html" frameborder="0" scrolling="no" width="745" height="584"></iframe>';
      localStorage.iFrameCnt = 1;
      break;

    //webServerSetting.php
    case 'pc':
      pc.style.backgroundColor = '#1D1E21';
      internalApp.style.backgroundColor = '#45464d';
      container.innerHTML = '<iframe src="./sysSetting/subCon1.html" frameborder="0" scrolling="no" width="745" height="584"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
    case 'internalApp':
      pc.style.backgroundColor = '#45464d';
      internalApp.style.backgroundColor = '#1D1E21';
      container.innerHTML = '<iframe src="./sysSetting/subCon1.html" frameborder="0" scrolling="no" width="745" height="584"></iframe>';
      localStorage.iFrameCnt = 1;
      break;
  }
}

function historyBack(){
  var cnt;
  var browser = navigator.userAgent.toLowerCase();

  if(localStorage.iFrameCnt){
    cnt = parseInt(localStorage.iFrameCnt, 10)
    cnt = -(1 + cnt);
    history.go(cnt);
    localStorage.removeItem("iFrameCnt");
  }else{
    if(browser.indexOf("chrome") != -1){
      var link = document.location.href;
      if(link.indexOf("#") != -1){
        history.go(-2);
      }else{
        history.back();
      }
    }else{
      history.back();
    }
  }
}
