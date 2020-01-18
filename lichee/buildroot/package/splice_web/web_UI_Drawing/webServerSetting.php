<?php
include('./loginChek.php')
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Web Server Setting</title>
    <link rel="stylesheet" href="userManagement.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('./topMenu1.html') ?>
      <div class="menuTitle homeToolBar" style="position:relative; left:2%;">
        <img src="./images/web_GUI/icon/webServerSetting.png"  width="32" height="32" style="position:relative;top:1.5%;left:0%;">
        WEB SERVER SETTING</div>
      <?php include('./topMenu2.html') ?>
      <div class="container">
        <div id="sideMenu">
          <ul>
            <li id="pc"  onclick="changePage('pc')">
              <a href="#">
                <div class="sideButton">
                  Web Server <-> PC
                </div>
              </a>
            </li>
            <li id="internalApp" onclick="changePage('internalApp')">
              <a href="#">
                <div class="sideButton">
                  Web Server <-> Internal App
                </div>
              </a>
            </li>
          </ul>
        </div>
        <div class="subPage" id="subPage">

        </div>
      </div>
      <div class="footClock foot">
          <?php include("./clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="images/web_GUI/icon2/home.png" width="32px" height="32px" style="position:relative; top: 10px;">
          Home Menu
          <img src="images/web_GUI/icon2/arrow.png" width="32px" height="16px" style="position:relative; top: 0px;">
          <img src="images/web_GUI/icon2/webServerSetting.png" width="32px" height="32px" style="position:relative; top: 10px;">
          <span style="color:red;">Web Server Setting</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("./footlogo.html") ?>
      </div>
    </div>
    <script src="./subPageControl.js"></script>
  </body>
</html>
