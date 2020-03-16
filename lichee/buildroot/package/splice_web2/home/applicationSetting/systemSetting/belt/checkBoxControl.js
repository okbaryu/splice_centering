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
