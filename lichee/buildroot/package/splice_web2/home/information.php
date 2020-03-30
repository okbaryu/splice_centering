<?php
include('../loginChek.php');
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="format-detection" content="telephone=no">
    <title>Information</title>
    <link rel="stylesheet" type="text/css" href="/font.css">
    <link rel="stylesheet" type="text/css" href="/home/information.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('../topMenu1.php') ?>
      <div class="menuTitle homeToolBar">
        <img src="../images/web_GUI/icon/information.png"  width="32px" height="32px" style="position:absolute;top:7.5%;left:32.5%;">
        INFORMATION</div>
        <?php include('../topMenu2.php') ?>
      <div class="container">

      </div>
      <div class="container1">
        <table class="info">
          <tr>
            <td class="tableTag">Web version</td>
            <td id="webversion"></td>
          </tr>
          <tr>
            <td class="tableTag">Splice centring version</td>
            <td id="spliceversion"></td>
          </tr>
          <tr>
            <td class="tableTag">Camera version</td>
            <td id=cameraversion></td>
          </tr>
          <tr>
            <td class="tableTag">Kernel version</td>
            <td id="UNAME"></td>
          </tr>
          <tr>
            <td class="tableTag">Server IP</td>
            <td id="serverip"></td>
          </tr>
          <tr>
            <td class="tableTag">Server time</td>
            <td id="splicetime"></td>
          </tr>
        </table>
      </div>
      <div class="footClock foot">
          <?php include("../clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="../images/web_GUI/icon2/home.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Home Menu</span>
          <img src="../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../images/web_GUI/icon2/information.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">Information</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("../footlogo.php") ?>
      </div>
    </div>
    <script src="../subPageControl.js"></script>
    <script src="../splice.js" ></script>
    <script>
      MyLoad()
      GetSysInfo.Value = 1;
      GetTime.Value = 1;
    </script>
  </body>
</html>
