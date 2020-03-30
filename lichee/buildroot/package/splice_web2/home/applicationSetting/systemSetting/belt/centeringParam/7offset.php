<!DOCTYPE html>
<html>
  <head>
    <title></title>
    <link rel="stylesheet" type="text/css" href="/font.css">
    <link rel="stylesheet" type="text/css" href="/home/applicationSetting/systemSetting/belt/centeringParam/7offset.css">
  </head>
  <body>
    <div class="container"><br>
      <img src="../../../../../images/web_GUI/web_GUI/bg_100.jpg" width="710px" height="540px" style="position:relative; top: -30px;">
      <div class="box">
        <ul style="position:relative; top: -5px; left: -25px;">
          <li class="label">Tip Detect Width</li>
          <li><input class="value_box" id="MWidth_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: -10px; left: -25px; width:230px;">
          <li class="label">1 Offset</li>
          <li><input class="value_box" id="LeadOffset0_val" type="text" name="" value="" disabled></li>
          <li class="label">2 Offset</li>
          <li><input class="value_box" id="LeadOffset1_val" type="text" name="" value="" disabled></li>
          <li class="label">3 Offset</li>
          <li><input class="value_box" id="LeadOffset2_val" type="text" name="" value="" disabled></li>
          <li class="label">4 Offset</li>
          <li><input class="value_box" id="LeadOffset3_val" type="text" name="" value="" disabled></li>
          <li class="label">5 Offset</li>
          <li><input class="value_box" id="LeadOffset4_val" type="text" name="" value="" disabled></li>
          <li class="label">6 Offset</li>
          <li><input class="value_box" id="LeadOffset5_val" type="text" name="" value="" disabled></li>
          <li class="label">7 Offset</li>
          <li><input class="value_box" id="LeadOffset6_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 5px; left: -25px;">
          <li class="label">Belt Material Offset (P)</li>
          <li><input class="value_box" id="P_Offset_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 10px; left: -25px;">
          <li class="label">Belt Width</li>
          <li><input class="value_box" id="GetSWidth_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 20px; left: -25px;">
          <li class="label">Belt Width Tolerance (+)</li>
          <li><input class="value_box" id="TolPos_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 20px; left: -25px;">
          <li class="label">Belt Width Tolerance (-)</li>
          <li><input class="value_box" id="TolNeg_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 25px; left: -25px; width:230px;">
          <li class="label">1 Offset</li>
          <li><input class="value_box" id="TrailOffset0_val" type="text" name="" value="" disabled></li>
          <li class="label">2 Offset</li>
          <li><input class="value_box" id="TrailOffset1_val" type="text" name="" value="" disabled></li>
          <li class="label">3 Offset</li>
          <li><input class="value_box" id="TrailOffset2_val" type="text" name="" value="" disabled></li>
          <li class="label">4 Offset</li>
          <li><input class="value_box" id="TrailOffset3_val" type="text" name="" value="" disabled></li>
          <li class="label">5 Offset</li>
          <li><input class="value_box" id="TrailOffset4_val" type="text" name="" value="" disabled></li>
          <li class="label">6 Offset</li>
          <li><input class="value_box" id="TrailOffset5_val" type="text" name="" value="" disabled></li>
          <li class="label">7 Offset</li>
          <li><input class="value_box" id="TrailOffset6_val" type="text" name="" value="" disabled></li>
        </ul>
      </div>
    </div>
    <script src="../../../../../splice.js"></script>
    <script>
      function init(){
        readCmdPLC();
        setInterval(readCmdPLC, 1000);
      }

      init();
    </script>
  </body>
</html>
