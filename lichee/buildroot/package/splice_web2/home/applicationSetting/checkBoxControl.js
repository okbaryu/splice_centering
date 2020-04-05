var fChecked = document.getElementsByName('functionSelect');
var button = new Array();
button[0] = document.getElementById('lBelt');
button[1] = document.getElementById('lThread');
button[2] = document.getElementById('lPabp');
button[3] = document.getElementById('lInner');

function linkDisable(){
  var id = new Array("lBelt", "lThread", "lPabp", "lInner")
  var link = new Array();
  link[0] = "location.href='./systemSetting/belt.php'";
  link[1] = "#";
  link[2] = "#";
  link[3] = "#";

  for(var i = 0; i < button.length; i++){
    if(fChecked[i].checked == true){
      button[i].style.backgroundColor = '#c0c0c0';
      button[i].style.cursor = 'pointer';
      button[i].setAttribute('onclick', `${link[i]}`);
      button[i].setAttribute('onmouseover', `hoverSet(${i})`)
      button[i].setAttribute('onmouseout', `hoverOut(${i})`)
    }else{
      button[i].style.backgroundColor = '#737373';
      button[i].style.cursor = '';
      button[i].setAttribute('onclick', '#');
      button[i].setAttribute('onmouseover', '')
      button[i].setAttribute('onmouseout', '')
    }
  }
}

function functionCheckbox(set){
  switch (set) {
    case 'belt':
      fChecked[0].checked = true;
      fChecked[1].checked = false;
      fChecked[2].checked = false;
      fChecked[3].checked = false;
      break;
    case 'thread':
      fChecked[0].checked = false;
      fChecked[1].checked = true;
      fChecked[2].checked = false;
      fChecked[3].checked = false;
      break;
    case 'pabp':
      fChecked[0].checked = false;
      fChecked[1].checked = false;
      fChecked[2].checked = true;
      fChecked[3].checked = false;
      break;
    case 'inner':
      fChecked[0].checked = false;
      fChecked[1].checked = false;
      fChecked[2].checked = false;
      fChecked[3].checked = true;
      break;
  }

  linkDisable()
}

function hoverSet(index){
  button[index].style.backgroundColor = '#1D1E21';
}

function hoverOut(index){
  button[index].style.backgroundColor = '#c0c0c0';
}

function encoderCheckbox(set){
  var eChecked = document.getElementsByName('encoderCheck');

  if(set == 'use'){
    eChecked[0].checked = true;
    eChecked[1].checked = false;
  }else{
    eChecked[0].checked = false;
    eChecked[1].checked = true;
  }
}

function actuatorCheckbox(set){
  var aChecked = document.getElementsByName('actCheck');
  var aValue = document.getElementById('direction_val');

  if(set == 'forward'){
    aChecked[0].checked = true;
    aChecked[1].checked = false;
    aValue.value = 0;
  }else{
    aChecked[0].checked = false;
    aChecked[1].checked = true;
    aValue.value = 1;
  }
}

function protocolSelect(set){
  var pChecked = document.getElementsByName('protocol');

  switch (set) {
    case 'ethernetUse':
      pChecked[0].checked = true;
      pChecked[1].checked = false;
      pChecked[2].checked = false;
      break;
    case '422Use':
      pChecked[0].checked = false;
      pChecked[1].checked = true;
      pChecked[2].checked = false;
      break;
    case '232Use':
      pChecked[0].checked = false;
      pChecked[1].checked = false;
      pChecked[2].checked = true;
      break;
  }
}
function TipDirctionSelect(set){
  var tChecked = document.getElementsByName('tipDirCheck');

  if(set == 'left'){
    tChecked[0].checked = true;
    tChecked[1].checked = false;
  }else{
    tChecked[0].checked = false;
    tChecked[1].checked = true;
  }
}
