<?php
include('./loginChek.php')
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>PLC Setting</title>
    <link rel="stylesheet" href="plcSetting.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('topMenu1.html') ?>
      <div class="menuTitle homeToolBar" style="position:relative; left:2%;">
        <img src="images/web_GUI/icon/plcDevice.png"  width="21" height="30" style="position:relative;top:1.5%;left:-1%;">
        PLC SETTING</div>
      <?php include('topMenu2.html') ?>
      <div class="container">
        <div id="sideMenu">
          <ul>
            <li  id="plcConnectSet"  onclick="changePage('plcConnectSet')">
              <a href="#">
                <div class="sideButton">
                  PLC Connection Setting
                </div>
              </a>
            </li>
            <li id="protocolSet" onclick="changePage('protocolSet')">
              <a href="#">
                <div class="sideButton">
                  Protocol Setting
                </div>
              </a>
            </li>
            <li id="plcDataLen" onclick="changePage('plcDataLen')">
              <a href="#">
                <div class="sideButton">
                  PLC Data Length
                </div>
              </a>
            </li>
            <li id="plcDataMonit" onclick="changePage('plcDataMonit')">
              <a href="#">
                <div class="sideButton">
                  PLC Data Monitoring
                </div>
              </a>
            </li>
          </ul>
        </div>
        <div class="subPage" id="subPage">

        </div>
      </div>
      <div class="footClock foot">
          <?php include("clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="images/web_GUI/icon2/home.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Home Menu</span>
          <img src="images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="images/web_GUI/icon2/deviceSetting.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Application Setting</span>
          <img src="images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="images/web_GUI/icon2/plcDevice.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">PLC Setting</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("footlogo.html") ?>
      </div>
    </div>
    <script src="./subPageControl.js"></script>
  </body>
</html>
