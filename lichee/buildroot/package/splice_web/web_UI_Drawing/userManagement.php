<?php  ?>
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
      <div class="container">
        <div id="sideMenu">
          <ul>
            <li  id="administrator"  onclick="changePage('administrator')">
              <a href="#">
                <div class="sideButton">
                  Administrator
                </div>
              </a>
            </li>
            <li id="maintenance" onclick="changePage('maintenance')">
              <a href="#">
                <div class="sideButton">
                  Maintenance
                </div>
              </a>
            </li>
            <li id="operator" onclick="changePage('operator')">
              <a href="#">
                <div class="sideButton">
                  Operator
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
          <img src="images/web_GUI/icon2/account.png" width="32px" height="32px" style="position:relative; top: 10px;">
          Account
          <img src="images/web_GUI/icon2/arrow.png" width="32px" height="16px" style="position:relative; top: 0px;">
          <img src="images/web_GUI/icon2/account.png" width="32px" height="32px" style="position:relative; top: 10px;">
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
