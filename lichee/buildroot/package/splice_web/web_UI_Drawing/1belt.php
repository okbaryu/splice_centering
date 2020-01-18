<?php
include('./loginChek.php')
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>1 Belt</title>
    <link rel="stylesheet" href="belt.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('topMenu1.html') ?>
      <div class="menuTitle homeToolBar" style="position:relative; left:2%;">
        <img src="images/web_GUI/icon/belt.png"  width="40" height="21" style="position:relative;top:1.5%;left:-1%;">
        1 BELT</div>
      <?php include('topMenu2.html') ?>
      <div class="container">
        <div id="sideMenu">
          <ul>
            <li  id="beltButton"  onclick="changePage('beltButton')">
              <a href="#">
                <div class="sideButton">
                  <img class="icon" src="images/web_GUI/icon/belt.png">
                  <span style="position:relative;right:10%;">Belt</span>
                  <img class="thumnail" src="images/web_GUI/web_GUI/belt.bmp">
                </div>
              </a>
            </li>
            <li id="threadButton" onclick="changePage('threadButton')">
              <a href="#">
                <div class="sideButton">
                  <img class="icon" src="images/web_GUI/icon/thread.png">
                  <span>Thread</span>
                  <img class="thumnail" src="images/web_GUI/web_GUI/thread.bmp" >
                </div>
              </a>
            </li>
            <li id="carcassButton" onclick="changePage('carcassButton')">
              <a href="#">
                <div class="sideButton">
                  <img class="icon" src="images/web_GUI/icon/carcass.png">
                  <span>Carcass</span>
                  <img class="thumnail" src="images/web_GUI/web_GUI/carcass.bmp">
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
          <img src="images/web_GUI/icon2/home.png" width="32px" height="32px" style="position:relative; top: 10px;">
          Home Menu
          <img src="images/web_GUI/icon2/arrow.png" width="32px" height="16px" style="position:relative; top: 0px;">
          <img src="images/web_GUI/icon2/deviceSetting.png" width="32px" height="32px" style="position:relative; top: 10px;">
          Application Setting
          <img src="images/web_GUI/icon2/arrow.png" width="32px" height="16px" style="position:relative; top: 0px;">
          <img src="images/web_GUI/icon2/systemSetting.png" width="32px" height="32px" style="position:relative; top: 10px;">
          System Setting
          <img src="images/web_GUI/icon2/arrow.png" width="32px" height="16px" style="position:relative; top: 0px;">
          <img src="images/web_GUI/icon2/Belt.png" width="32px" height="32px" style="position:relative; top: 10px;">
          <span style="color:red;">1 Belt</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("footlogo.html") ?>
      </div>
    </div>
    <script src="./subPageControl.js"></script>
  </body>
</html>
