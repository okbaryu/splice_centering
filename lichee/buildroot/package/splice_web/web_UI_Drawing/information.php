<?php  ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Information</title>
    <link rel="stylesheet" href="Information.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('topMenu1.html') ?>
      <div class="menuTitle homeToolBar">
        <img src="images/web_GUI/icon/information.png"  width="32px" height="32px" style="position:absolute;top:7.5%;left:32.5%;">
        INFORMATION</div>
        <?php include('topMenu2.html') ?>
      <div class="container">
        <div class="container1">

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
          <img src="images/web_GUI/icon2/information.png" width="32px" height="32px" style="position:relative; top: 10px;">
          <span style="color:red;">Information</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("footlogo.html") ?>
      </div>
    </div>
  </body>
</html>
