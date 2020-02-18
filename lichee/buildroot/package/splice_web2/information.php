<?php
include('./loginChek.php');
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Information</title>
    <link rel="stylesheet" href="information.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('topMenu1.html') ?>
      <div class="menuTitle homeToolBar">
        <img src="images/web_GUI/icon/information.png"  width="32px" height="32px" style="position:absolute;top:7.5%;left:32.5%;">
        INFORMATION</div>
        <?php include('topMenu2.html') ?>
      <div class="container">

      </div>
      <div class="container1">
        <table class="info">
          <tr>
            <td class="tableTag">S/W Version</td>
            <td></td>
          </tr>
          <tr>
            <td class="tableTag">Runtime Data</td>
            <td></td>
          </tr>
          <tr>
            <td class="tableTag">Network Information</td>
            <td></td>
          </tr>
          <tr>
            <td class="tableTag">System Status</td>
            <td></td>
          </tr>
        </table>
      </div>
      <div class="footClock foot">
          <?php include("./clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="images/web_GUI/icon2/home.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Home Menu</span>
          <img src="images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="images/web_GUI/icon2/information.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">Information</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("footlogo.html") ?>
      </div>
    </div>
    <script src="./subPageControl.js"></script>
  </body>
</html>
