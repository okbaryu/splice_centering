<?php
include('../loginChek.php');
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Web Server Setting</title>
    <link rel="stylesheet" href="webServerSetting.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('../topMenu1.php') ?>
      <div class="menuTitle homeToolBar" style="position:relative; left:2%;">
        <img src="../images/web_GUI/icon/webServerSetting.png"  width="32" height="32" style="position:relative;top:1.5%;left:0%;">
        WEB SERVER SETTING</div>
      <?php include('../topMenu2.php') ?>
      <div class="container">
        <div id="sideMenu">
          <ul>
            <li id="sSetting" onclick="changePage('sSetting')">
              <a href="#">
                <div class="sideButton">
                  Setting
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
          <?php include("../clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="../images/web_GUI/icon2/home.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Home Menu</span>
          <img src="../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../images/web_GUI/icon2/webServerSetting.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">Web Server Setting</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("../footlogo.php") ?>
      </div>
    </div>
    <script src="../subPageControl.js"></script>
  </body>
</html>
