<?php
include('./loginChek.php');
 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Home Menu</title>
    <link rel="stylesheet" href="homeMenu.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('topMenu1.html') ?>
      <div class="menuTitle homeToolBar">
        <img src="images/web_GUI/icon/home.png"  width="32px" height="32px" style="position:absolute;top:7.5%;left:33.5%;">
        HOME MENU</div>
        <?php include('topMenu2.html') ?>
      <div class="container">
        <div class="container1">
          <ul>
            <li>
              <a href="#">
                <table onclick="location.href='./applicationSetting.php'">
                  <tr>
                    <td><img class="linkImg" src="images/web_GUI/icon/deviceSetting.png" width="100px" height="100px"></td>
                  </tr>
                  <tr>
                    <td class="menuText">Application Setting</td>
                  </tr>
                </table>
              </a>
            </li>
            <li>
              <a href="#">
                <table onclick="location.href='./account.php'">
                  <tr>
                    <td><img id="account" class="linkImg" src="images/web_GUI/icon/account.png" width="100px" height="100px"></td>
                  </tr>
                  <tr>
                    <td class="menuText">Account</td>
                  </tr>
                </table>
              </a>
            </li>
            <li>
              <a href="#">
                <table onclick="location.href='./information.php'">
                  <tr>
                    <td><img class="linkImg" src="images/web_GUI/icon/information.png" width="100px" height="100px"></td>
                  </tr>
                  <tr>
                    <td class="menuText">Information</td>
                  </tr>
                </table>
              </a>
            </li>
            <li>
              <a href="#">
                <table  onclick="location.href='./webServerSetting.php'">
                  <tr>
                    <td><img class="linkImg" src="images/web_GUI/icon/webServerSetting.png" width="100px" height="100px"></td>
                  </tr>
                  <tr>
                    <td class="menuText">Web Server Setting</td>
                  </tr>
                </table>
              </a>
            </li>
          </ul>
        </div>
      </div>
      <div class="footClock foot">
          <?php include("clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="images/web_GUI/icon2/home.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">Home Menu</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("footlogo.html") ?>
      </div>
    </div>
    <script src="./subPageControl.js"></script>
  </body>
</html>
