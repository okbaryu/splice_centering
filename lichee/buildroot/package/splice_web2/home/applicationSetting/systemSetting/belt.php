<?php
include('../../../loginChek.php');
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Belt</title>
    <link rel="stylesheet" type="text/css" href="/font.css">
    <link rel="stylesheet" type="text/css" href="/home/applicationSetting/systemSetting/belt.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('../../../topMenu1.php') ?>
      <div class="menuTitle homeToolBar" style="position:relative; left:2%;">
        <img src="../../../images/web_GUI/icon/belt.png"  width="40" height="21" style="position:relative;top:1.5%;left:-1%;">
        BELT
      </div>
      <?php include('../../../topMenu2.php') ?>
      <div class="container">
        <div id="sideMenu">
          <ul>
            <li id="sCenteringParam"  onclick="changePage('sCenteringParam')">
              <a href="#">
                <div class="sideButton">
                  Centering Parameter
                </div>
              </a>
            </li>
            <li id="sActuator" onclick="changePage('sActuator')">
              <a href="#">
                <div class="sideButton">
                  Actuator
                </div>
              </a>
            </li>
            <li id="sCamera" onclick="changePage('sCamera')">
              <a href="#">
                <div class="sideButton">
                  Camera
                </div>
              </a>
            </li>
            <li id="sEncoder" onclick="changePage('sEncoder')">
              <a href="#">
                <div class="sideButton">
                  Encoder
                </div>
              </a>
            </li>
            <li id="sSystemOverview" onclick="changePage('sSystemOverview')">
              <a href="#">
                <div class="sideButton">
                  System Overview
                </div>
              </a>
            </li>
            <li id="sLogData" onclick="changePage('sLogData')">
              <a href="#">
                <div class="sideButton">
                  Log Data
                </div>
              </a>
            </li>
          </ul>
        </div>
        <div id="effect">
          <ul>
            <li id="eBlock1"></li>
            <li id="eBlock2"></li>
            <li id="eBlock3"></li>
            <li id="eBlock4"></li>
            <li id="eBlock5"></li>
            <li id="eBlock6"></li>
          </ul>
        </div>
        <div id="subPage">

        </div>
      </div>
      <div class="footClock foot">
          <?php include("../../../clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="../../../images/web_GUI/icon2/home.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Home Menu</span>
          <img src="../../../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../../../images/web_GUI/icon2/deviceSetting.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Application Setting</span>
          <img src="../../../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../../../images/web_GUI/icon2/systemSetting.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">System Setting</span>
          <img src="../../../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../../../images/web_GUI/icon2/belt.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">Belt</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("../../../footlogo.php") ?>
      </div>
    </div>
    <script src="../../../subPageControl.js"></script>
    <script>
      changePage('sCenteringParam');
    </script>
  </body>
</html>
