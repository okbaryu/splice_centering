<?php
include('../../loginChek.php');
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>PLC Setting</title>
    <link rel="stylesheet" type="text/css" href="/font.css">
    <link rel="stylesheet" type="text/css" href="/home/applicationSetting/plcSetting.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('../../topMenu1.php') ?>
      <div class="menuTitle homeToolBar" style="position:relative; left:2%;">
        <img src="../../images/web_GUI/icon/plcDevice.png"  width="32" height="29" style="position:relative;top:1.5%;left:-1%;">
        PLC Setting
      </div>
      <?php include('../../topMenu2.php') ?>
      <div class="container">
        <div id="sideMenu">
          <ul>
            <li id="sConnection"  onclick="changePage('sConnection')">
              <a href="#">
                <div class="sideButton">
                  Connection
                </div>
              </a>
            </li>
            <li id="sDataMonitor" onclick="changePage('sDataMonitor')">
              <a href="#">
                <div class="sideButton">
                  Data Monitor
                </div>
              </a>
            </li>
          </ul>
        </div>
        <div id="effect">
          <ul>
            <li id="eBlock1"></li>
            <li id="eBlock2"></li>
          </ul>
        </div>
        <div id="subPage">

        </div>
      </div>
      <div class="footClock foot">
          <?php include("../../clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="../../images/web_GUI/icon2/home.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Home Menu</span>
          <img src="../../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../../images/web_GUI/icon2/deviceSetting.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Application Setting</span>
          <img src="../../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../../images/web_GUI/icon2/plcDevice.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">PLC Setting</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("../../footlogo.php") ?>
      </div>
    </div>
    <script src="../../subPageControl.js"></script>
    <script>
      changePage('sConnection');
    </script>
  </body>
</html>
