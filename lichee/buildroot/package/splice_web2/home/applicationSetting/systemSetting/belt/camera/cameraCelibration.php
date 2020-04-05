<?php
 ?>
<!DOCTYPE html>
<html>
  <head>
    <title></title>
    <link rel="stylesheet" type="text/css" href="/font.css">
    <link rel="stylesheet" type="text/css" href="/home/applicationSetting/systemSetting/belt/camera/cameraCelibration.css">
  </head>
  <body>
    <div class="container">
      <table style="position: relative;top: -100px;left: 30px;">
        <tr>
          <td>
            <input type="checkbox" id="caliMod" name="caliModSelect" class="cb" onclick="caliSelect('cali')">
            <label for="caliMod"></label>
            <span class="label_text">Calibration Mods</span>
          </td>
        </tr>
        <tr>
          <td>
            <input type="checkbox" id="cam0" name="caliModSelect" class="cb" onclick="caliSelect('cam0')">
            <label for="cam0"></label>
            <span class="label_text">Cam 0 Select</span>
          </td>
        </tr>
        <tr>
          <td>
            <input type="checkbox" id="cam1" name="caliModSelect" class="cb" onclick="caliSelect('cam1')">
            <label for="cam1"></label>
            <span class="label_text">Cam 1 Select</span>
          </td>
        </tr>
        <tr>
          <td>
            <input type="checkbox" id="nomalMod" name="caliModSelect" class="cb" onclick="caliSelect('nomal')">
            <label for="nomalMod"></label>
            <span class="label_text">Nomal Mods</span>
          </td>
        </tr>
      </table>
      <input type="hidden" class="saveButton cam0" id="Cam0" value="Save Calibration" onclick="caliSelect('sCam0')">
      <input type="hidden" class="saveButton cam1" id="Cam1" value="Save Calibration" onclick="caliSelect('sCam0')">
    </div>
    <script src="../../../checkBoxControl.js"></script>
    <script src="../../../../../splice.js"></script>\
    <script>
       caliSelect('onload');
    </script>
  </body>
</html>
