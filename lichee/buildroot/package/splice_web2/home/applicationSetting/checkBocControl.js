function functionCheckbox(set){
  var fChecked = document.getElementsByName('functionSelect')

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
    default:

  }
}
