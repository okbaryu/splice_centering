<?php  ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title></title>
    <link rel="stylesheet" type="text/css" href="/font.css">
    <link rel="stylesheet" type="text/css" href="/home/applicationSetting/systemSetting/belt/camera.css">
  </head>
  <body>
    <div class="button">
      <ul>
        <li id="sCalibration" onclick="changePage('sCalibration')">
          <a href="#">
            Calibration
          </a>
        </li>
        <li id="sCameraStatus" onclick="changePage('sCameraStatus')">
          <a href="#">
            Camera Status
          </a>
        </li>
      </ul>
    </div>
    <div class="container" id="subPage">

    </div>
    <script src="../../../../subPageControl.js"></script>
    <script src="../../../../splice.js"></script>
    <script>
      changePage('sCalibration');
    </script>
    <script type="text/javascript">
      window.onbeforeunload = function() {
      caliSelect('nomal');
    }
    </script>
  </body>
</html>
