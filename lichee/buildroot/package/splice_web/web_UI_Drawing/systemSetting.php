<?php  ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>System Setting</title>
    <link rel="stylesheet" href="systemSetting.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('topMenu1.html') ?>
      <div class="menuTitle homeToolBar">
        <img src="images/web_GUI/icon/systemsetting.png"  width="32px" height="32px" style="position:absolute;top:7.5%;left:30.5%;">
        SYSTEM SETTING</div>
      <?php include('topMenu2.html') ?>
      <div class="container">
        <div class="container1">
          <ul>

            <li>
              <a href="#">
                <table onclick="location.href='./1belt.php'">
                  <tr>
                    <td><img class="linkImg" src="images/web_GUI/icon/belt.png" width="100px" height="100px"></td>
                  </tr>
                  <tr>
                    <td class="menuText">1 Belt</td>
                  </tr>
                </table>
              </a>
            </li>
            <li>
              <a href="#">
                <table onclick="location.href='./2belt.php'">
                  <tr>
                    <td><img class="linkImg" src="images/web_GUI/icon/belt.png" width="100px" height="100px"></td>
                  </tr>
                  <tr>
                    <td class="menuText">2 Belt</td>
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
          <img src="images/web_GUI/icon2/home.png" width="32px" height="32px" style="position:relative; top: 10px;">
          Home Menu
          <img src="images/web_GUI/icon2/arrow.png" width="32px" height="16px" style="position:relative; top: 0px;">
          <img src="images/web_GUI/icon2/deviceSetting.png" width="32px" height="32px" style="position:relative; top: 10px;">
          Application Setting
          <img src="images/web_GUI/icon2/arrow.png" width="32px" height="16px" style="position:relative; top: 0px;">
          <img src="images/web_GUI/icon2/systemSetting.png" width="32px" height="32px" style="position:relative; top: 10px;">
          <span style="color:red;">System Setting</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("footlogo.html") ?>
      </div>
    </div>
  </body>
</html>
