<?php  ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title></title>
    <link rel="stylesheet" href="./beltSub.css">
  </head>
  <body>
    <div class="button">
      <ul>
        <li id="cpsButton" onclick="changePage('cpsButton')">
          <a href="#">
            <img src="../images/web_GUI/icon/paramSetting.png" width=35px; height=25px; style="margin-left: 3px;margin-right: 7px;float:left;">
            Centering parameter Setting
          </a>
        </li>
        <li id="asButton" onclick="changePage('asButton')">
          <a href="#">
            <img src="../images/web_GUI/icon/acctuatorSetting.png" width=35px; height=25px; style="margin-left: 3px;margin-right: 7px;float:left;">
            Actuator Setting
          </a>
        </li>
        <li id="csButton" onclick="changePage('csButton')">
          <a href="#">
            <img src="../images/web_GUI/icon/camSetting.png" width=35px; height=25px; style="margin-left: 3px;margin-right: 7px;float:left;">
            Camera Setting
          </a>
        </li>
      </ul>
    </div>
    <div class="container" id="subPage">

    </div>
    <script src="./../subPageControl.js"></script>
  </body>
</html>
