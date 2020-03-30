<?php
  include('../loginChek.php');
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Account</title>
    <link rel="stylesheet" type="text/css" href="/font.css">
    <link rel="stylesheet" type="text/css" href="/home/account.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('../topMenu1.php') ?>
      <div class="menuTitle homeToolBar">
        <img src="../images/web_GUI/icon/account.png"  width="32px" height="32px" style="position:absolute;top:7.5%;left:35.5%;">
        ACCOUNT</div>
        <?php include('../topMenu2.php') ?>
      <div class="container">
        <div class="container1">
          <ul>
            <li>
              <a href="#">
                <table onclick="location.href='./account/userManagement.php'">
                  <tr>
                    <td><img id="account" class="linkImg" src="../images/web_GUI/icon/account.png" width="100px" height="100px"></td>
                  </tr>
                  <tr>
                    <td class="menuText">User Management</td>
                  </tr>
                </table>
              </a>
            </li>
            <li>
              <a href="#">
                <table onclick="location.href='../login.php'">
                  <tr>
                    <td><img class="linkImg" src="../images/web_GUI/icon/passwd.png" width="100px" height="100px"></td>
                  </tr>
                  <tr>
                    <td class="menuText">Login/Logout</td>
                  </tr>
                </table>
              </a>
            </li>
          </ul>
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
          <img src="../images/web_GUI/icon2/account.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">Account</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("../footlogo.php") ?>
      </div>
    </div>
    <script src="../subPageControl.js"></script>
  </body>
</html>
