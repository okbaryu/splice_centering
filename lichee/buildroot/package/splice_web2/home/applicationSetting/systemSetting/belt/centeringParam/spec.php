<!DOCTYPE html>
<html>
  <head>
    <title></title>
    <link rel="stylesheet" href="./spec.css">
  </head>
  <body>
    <div class="container"><br>
      <img src="../../../../../images/web_GUI/web_GUI/06_background img.gif" width="710px" height="540px" style="position:relative; top: -30px;">
      <ul style="position:relative; top: -520px; left: 230px;">
        <li class="label">Conveyor Offset (M)</li>
        <li class="value_box">0.0 mm</li>
      </ul>
      <ul style="position:relative; top: -400px; left: 230px;">
        <li class="label">Belt Width</li>
        <li class="value_box">0.0 mm</li>
        <li class="label">Belt Width Tolerance (+)</li>
        <li class="value_box">0.0 mm</li>
        <li class="label">Belt Width Tolerance (-)</li>
        <li class="value_box">0.0 mm</li>
      </ul>
      <div class="direction box" style="position:relative; top:-750px; left: 440px;">
        <div style="color:white; font-size:18px; position:relative; left:40px; top:20px;">Leading Tip Direction</div>
        <table style="position:relative; top:60px; left:8px; z-index:5">
          <tr>
            <td>
              <input type="checkbox" id="left" name="leftCheck" class="cb" value="left" disabled="disabled">
              <label for="left"></label>
            </td>
            <td class="cb_label">
              Tip Left
            </td>
          </tr>
        </table>
        <table style="position:relative; top:31px; left:158px; z-index:5">
          <tr>
            <td>
              <input type="checkbox" id="right" name="righttCheck" class="cb" value="right" disabled="disabled">
              <label for="right"></label>
            </td>
            <td class="cb_label">
              Tip Right
            </td>
          </tr>
        </table>
        <img src="../../../../../images/web_GUI/web_GUI/06-system_belt-direction.png" style="position:relative; top:-30px; left:81px;">
      </div>
      <div class="method box" style="position:relative; top:-720px; left: 440px;">
        <div style="color:white; font-size:18px; position:relative; left:40px; top:20px;">Belt Guiding Method</div>
        <table style="position:relative; top:31px; left:10px; z-index:5">
          <tr>
            <td>
              <input type="checkbox" id="right" name="righttCheck" class="cb" value="right" disabled="disabled">
              <label for="right"></label>
            </td>
            <td class="cb_label">
              Standard Guiding
            </td>
          </tr>
        </table>
        <table style="position:relative; top:31px; left:10px; z-index:5">
          <tr>
            <td>
              <input type="checkbox" id="right" name="righttCheck" class="cb" value="right" disabled="disabled">
              <label for="right"></label>
            </td>
            <td class="cb_label">
              7 Offset Guiding
            </td>
          </tr>
        </table>
        <table style="position:relative; top:31px; left:10px; z-index:5">
          <tr>
            <td>
              <input type="checkbox" id="right" name="righttCheck" class="cb" value="right" disabled="disabled">
              <label for="right"></label>
            </td>
            <td class="cb_label">
              Auto Splice Guiding
            </td>
          </tr>
        </table>
      </div>
    </div>
  </body>
</html>