<?php
include('./loginChek.php')
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Application Setting</title>
    <link rel="stylesheet" href="applicationsetting.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('topMenu1.html') ?>
      <div class="menuTitle homeToolBar">
        <img src="images/web_GUI/icon/deviceSetting.png"  width="32px" height="32px" style="position:absolute;top:7.5%;left:26.5%;">
        APPLICATION SETTING</div>
        <?php include('topMenu2.html') ?>
      <div class="container">
        <div class="container1">
          <ul>
            <li>
              <a href="#">
                <div class="inContainer" onclick="location.href='./systemsetting.php'">
                  <div id="ractangel" onmouseover="changeColor()" onmouseout="resetColor()">
                    <img id="account" class="linkImg" src="images/web_GUI/icon/systemSetting.png" width="100px" height="100px" style="position:relative; top:30px;">
                  </div><!-- -->
                  <div id="triangle" onmouseover="changeColor()" onmouseout="resetColor()">

                  </div><!-- -->
                  <span class="textB sysText">System Setting</span>
                </div>
              </a>
            </li>
            <li>
              <a href="#">
                <div class="inContainer1" onclick="location.href='./plcSetting.php'">
                  <div id="ractangel1" onmouseover="changeColor1()" onmouseout="resetColor1()">
                    <img class="linkImg" src="images/web_GUI/icon/plcDevice.png" width="100px" height="100px" style="position:relative; top:30px;">
                  </div><!-- -->
                  <div id="triangle1" onmouseover="changeColor1()" onmouseout="resetColor1()">

                  </div><!-- -->
                  <span class="textB plcText">PLC Setting</span>
                </div>
              </a>
            </li>
          </ul>
        </div>
        <div class="block"></div>
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
          <span style="color:red;">Application Setting</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("footlogo.html") ?>
      </div>
    </div>
    <script src="./subPageControl.js"></script>
    <script src="./blockColorSet.js"></script>
  </body>
</html>
