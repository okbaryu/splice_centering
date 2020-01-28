<?php
include('./loginChek.php')
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>User Management</title>
    <link rel="stylesheet" href="userManagement.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('./topMenu1.html') ?>
      <div class="menuTitle homeToolBar" style="position:relative; left:2%;">
        <img src="./images/web_GUI/icon/account.png"  width="32" height="32" style="position:relative;top:1.5%;left:-1%;">
        USER MANAGEMENT</div>
      <?php include('./topMenu2.html') ?>
      <div class="container"></div>
      <div class="subPage" id="subPage"></div>
      <div class="subTool">
        <table style="border-spacing: 0;">
          <tr>
            <td style="width: 128px; height: 48px; color:white; text-align:center;">ID</td>
            <td style="width: 192px; height: 48px; color:white; text-align:center;">User Name</td>
            <td style="width: 320px; height: 48px; color:white; text-align:center;">E-Mail</td>
            <td style="width: 128px; height: 48px; color:white; text-align:center;">Role</td>
            <td style="width: 128px; height: 48px; color:white; text-align:center;">Action</td>
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
          <img src="images/web_GUI/icon2/account.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Account</span>
          <img src="images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="images/web_GUI/icon2/account.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">User Management</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("./footlogo.html") ?>
      </div>
    </div>
    <script src="./subPageControl.js"></script>
  </body>
</html>
