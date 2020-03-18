<?php
include('../../loginChek.php');
?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>System Setting</title>
    <link rel="stylesheet" href="systemSetting.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('../../topMenu1.php') ?>
      <div class="menuTitle homeToolBar">
        <img src="../../images/web_GUI/icon/systemSetting.png"  width="32px" height="32px" style="position:absolute;top:7.5%;left:30.5%;">
        SYSTEM SETTING</div>
      <?php include('../../topMenu2.php') ?>
      <div class="container">
        <div class="container1">
          <ul>
            <li>
              <table>
                <tr>
                  <td class="tb_box" id="lBelt" onclick="location.href='./systemSetting/belt.php'">
                    <table style="width:110px; height:60px; margin:auto;">
                      <tr>
                        <td>
                        <img class="linkImg" src="../../images/web_GUI/icon/belt.png" width="100px" height="50px">
                        </td>
                      </tr>
                      <tr>
                        <td class="tlabel">Belt</td>
                      </tr>
                    </table>
                  </td>
                </tr>
                <tr>
                  <td>
                    <div class="menuCheckBox">
                      <input type="checkbox" name="functionSelect" id="checkBelt" class="cb" onclick="functionCheckbox('belt')" checked="checked">
                      <label for="checkBelt"></label>
                    </div>
                  </td>
                </tr>
              </table>
            </li>
            <li>
              <table>
                <tr>
                  <td class="tb_box" id="lThread" onclick="location.href='#'">
                    <table style="width:110px; height:60px; margin:auto;">
                      <tr>
                        <td>
                        <img class="linkImg" src="../../images/web_GUI/icon/thread.png" width="100px" height="50px">
                        </td>
                      </tr>
                      <tr>
                        <td class="tlabel">Thread</td>
                      </tr>
                    </table>
                  </td>
                </tr>
                <tr>
                  <td>
                    <div class="menuCheckBox">
                      <input type="checkbox" name="functionSelect" id="checkThread" class="cb" onclick="functionCheckbox('thread')">
                      <label for="checkThread"></label>
                    </div>
                  </td>
                </tr>
              </table>
            </li>
            <li>
              <table>
                <tr>
                  <td class="tb_box" id="lPabp" onclick="location.href='#'">
                    <table style="width:110px; height:60px; margin:auto;">
                      <tr>
                        <td class="tlabel" style="font-size:40px;">?
                        <!-- <img class="linkImg" src="#" width="100px" height="50px"> -->
                        </td>
                      </tr>
                      <tr>
                        <td class="tlabel">PA/BP</td>
                      </tr>
                    </table>
                  </td>
                </tr>
                <tr>
                  <td>
                    <div class="menuCheckBox">
                      <input type="checkbox" name="functionSelect" id="checkPabp" class="cb" onclick="functionCheckbox('pabp')">
                      <label for="checkPabp"></label>
                    </div>
                  </td>
                </tr>
              </table>
            </li>
            <li>
              <table>
                <tr>
                  <td class="tb_box" id="lInner" onclick="location.href='#'">
                    <table style="width:110px; height:60px; margin:auto;">
                      <tr>
                        <td class="tlabel" style="font-size:40px;">!
                          <!-- <img class="linkImg" src="#" width="100px" height="50px"> -->
                        </td>
                      </tr>
                      <tr>
                        <td class="tlabel">Inner</td>
                      </tr>
                    </table>
                  </td>
                </tr>
                <tr>
                  <td>
                    <div class="menuCheckBox">
                      <input type="checkbox" name="functionSelect" id="checkInner" class="cb" onclick="functionCheckbox('inner')">
                      <label for="checkInner"></label>
                    </div>
                  </td>
                </tr>
              </table>
            </li>
          </ul>
          <input type="submit" id="sysSubmit" name="systemSubmit" value="Apply">
        </div>
      </div>
      <div class="footClock foot">
          <?php include("../../clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="../../images/web_GUI/icon2/home.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Home Menu</span>
          <img src="../../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../../images/web_GUI/icon2/deviceSetting.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Application Setting</span>
          <img src="../../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../../images/web_GUI/icon2/systemSetting.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">System Setting</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("../../footlogo.htmphp") ?>
      </div>
    </div>
    <script src="./checkBoxControl.js"></script>
    <script src="../../subPageControl.js"></script>
    <script type="text/javascript">
      (function(){
        linkDisable()
      })()
    </script>
  </body>
</html>
