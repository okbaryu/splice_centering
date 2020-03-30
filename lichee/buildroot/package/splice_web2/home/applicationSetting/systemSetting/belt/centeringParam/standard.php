<!DOCTYPE html>
<html>
  <head>
    <title></title>
    <link rel="stylesheet" type="text/css" href="/font.css">
    <link rel="stylesheet" type="text/css" href="/home/applicationSetting/systemSetting/belt/centeringParam/standard.css">
  </head>
  <body>
    <div class="container"><br>
      <img src="../../../../../images/web_GUI/web_GUI/07_background img.jpg" width="710px" height="540px" style="position:relative; top: -30px;">
      <div class="box">
        <ul style="position:relative; top: -5px; left: -25px;">
          <li class="label">Tip Detect Width</li>
          <li><input class="value_box" id="MWidth_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 10px; left: -25px;">
          <li class="label">Leading Tip edge Offset</li>
          <li><input class="value_box" id="OffsetIn_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 10px; left: -25px;">
          <li class="label">Tip Offset --> Tip Guiding Width</li>
          <li><input class="value_box" id="SWidthIn_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 70px; left: -25px;">
          <li class="label">Belt Material Offset (P)</li>
          <li><input class="value_box" id="P_Offset_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 110px; left: -25px;">
          <li class="label">Belt Width</li>
          <li><input class="value_box" id="GetSWidth_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 120px; left: -25px;">
          <li class="label">Belt Width Tolerance (+)</li>
          <li><input class="value_box" id="TolPos_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 120px; left: -25px;">
          <li class="label">Belt Width Tolerance (-)</li>
          <li><input class="value_box" id="TolNeg_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 195px; left: -25px;">
          <li class="label">Tip Guiding --> Tip Offset Width</li>
          <li><input class="value_box" id="SWidthOut_val" type="text" name="" value="" disabled></li>
        </ul>
        <ul style="position:relative; top: 200px; left: -25px;">
          <li class="label">Trailling Tip edge Offset</li>
          <li><input class="value_box" id="OffsetOut_val" type="text" name="" value="" disabled></li>
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
