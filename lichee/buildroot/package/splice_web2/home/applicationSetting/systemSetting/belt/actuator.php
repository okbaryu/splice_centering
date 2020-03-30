<?php
 ?>
 <!DOCTYPE html>
 <html>
   <head>
     <meta charset="utf-8">
     <title></title>
    <link rel="stylesheet" type="text/css" href="/font.css">
     <link href="/base/jquery-ui.min.css" type="text/css" rel="stylesheet">
     <link href="/splice.css" type="text/css" rel="stylesheet">
     <link href="/home/applicationSetting/systemSetting/belt/actuator.css" type="text/css" rel="stylesheet">
     <script src="/jquery-1.12.4.min.js"></script>
     <script src="/jquery-ui.min.js"></script>
     <script src="/wb.validation.min.js"></script>
     <script src="/wwb15.min.js"></script>
    <script>
    $(document).ready(function()
    {
       $("#Slider1").slider(
       {
          orientation: 'horizontal',
          animate: true,
          range: 'min',
          min: -50,
          max: 50,
          value: 0
       });

       $("#Slider1").bind('slidechange', function(event, ui)
       {
    SliderCB(event, ui)
       });

       $("#Slider1").bind('slide', function(event, ui)
       {
     $("#calibar_val").val(ui.value);
       });
       $("#Slider1").tooltip(
       {
          hide: true,
          show: true,
          content: "",
          items: '#Slider1',
          position: { my: "right bottom", at: "left top", collision: "flipfit" },
          classes: { 'ui-tooltip' : 'ToolTip1' }
       });
       $("#stroke_val").validate(
       {
          required: true,
          bootstrap: true,
          type: 'number',
          expr_min: '>=',
          expr_max: '<=',
          value_min: '50',
          value_max: '100',
          color_text: '#000000',
          color_hint: '#00FF00',
          color_error: '#FF0000',
          color_border: '#808080',
          nohint: false,
          font_family: 'Arial',
          font_size: '13px',
          position: 'topleft',
          offsetx: 0,
          offsety: 0,
          effect: 'none',
          error_text: 'Input integer between 50 ~ 100'
       });
       $("#speed_val").validate(
       {
          required: true,
          bootstrap: true,
          type: 'number',
          expr_min: '>=',
          expr_max: '<=',
          value_min: '50',
          value_max: '100',
          color_text: '#000000',
          color_hint: '#00FF00',
          color_error: '#FF0000',
          color_border: '#808080',
          nohint: false,
          font_family: 'Arial',
          font_size: '13px',
          position: 'topleft',
          offsetx: 0,
          offsety: 0,
          effect: 'none',
          error_text: 'Input integer between 50 ~ 100'
       });
       $("#left_limit_val").validate(
       {
          required: true,
          bootstrap: true,
          type: 'number',
          expr_min: '>=',
          expr_max: '<=',
          value_min: '100',
          value_max: '5000',
          color_text: '#000000',
          color_hint: '#00FF00',
          color_error: '#FF0000',
          color_border: '#808080',
          nohint: false,
          font_family: 'Arial',
          font_size: '13px',
          position: 'topleft',
          offsetx: 0,
          offsety: 0,
          effect: 'none',
          error_text: 'Input integer between 100 ~ 5000'
       });
       $("#right_limit_val").validate(
       {
          required: true,
          bootstrap: true,
          type: 'number',
          expr_min: '>=',
          expr_max: '<=',
          value_min: '100',
          value_max: '5000',
          color_text: '#000000',
          color_hint: '#00FF00',
          color_error: '#FF0000',
          color_border: '#808080',
          nohint: false,
          font_family: 'Arial',
          font_size: '13px',
          position: 'topleft',
          offsetx: 0,
          offsety: 0,
          effect: 'none',
          error_text: 'Input integer between 100 ~ 5000'
       });
       $("#origin_val").validate(
       {
          required: true,
          bootstrap: true,
          type: 'number',
          expr_min: '>=',
          expr_max: '<=',
          value_min: '-2500',
          value_max: '2500',
          color_text: '#000000',
          color_hint: '#00FF00',
          color_error: '#FF0000',
          color_border: '#808080',
          nohint: false,
          font_family: 'Arial',
          font_size: '13px',
          position: 'topleft',
          offsetx: 0,
          offsety: 0,
          effect: 'none',
          error_text: 'Input integer between -2500 ~ 2500'
       });
    });
    </script>
   </head>
   <body>
     <div class="container">
       <table style="position:relative;left: 270px;top: 20px;">
         <tr>
           <td style="color:white; font-size:15px; text-align:center;">IP ADDRESS TO CONNECT</td>
         </tr>
         <tr>
           <td><input class="table_textbox" style="width: 180px;" type="text" name="ip_addr" id="ip_addr" value=""></td>
         </tr>
       </table>
       <ul style="position:relative;left: -20px;top: 30px;">
         <li class="actuatorBox1">
           <table style="position:relative; top: 5px; left: 225px;">
             <tr>
               <td style="color:white; font-size:15px; text-align:center;">Meterial Flow</td>
             </tr>
             <tr>
               <td style="color:white; font-size:15px; text-align:center;">Direction</td>
             </tr>
           </table>
           <table style="position:relative; left: 130px;top: 5px;">
             <tr>
               <td style="color:white; font-size:15px; text-align:center;">MAX STROKE</td>
             </tr>
             <tr>
               <td><input class="table_textbox" style="width: 100px;" type="text" name="stroke_val" id="stroke_val" value=""></td>
             </tr>
           </table>
           <table style="position:relative; top: -57px;left: 305px;">
             <tr>
               <td style="color:white; font-size:15px; text-align:center;">SPEED</td>
             </tr>
             <tr>
               <td><input class="table_textbox" style="width: 100px;" type="text" name="speed_val" id="speed_val" value=""></td>
             </tr>
           </table>
           <img src="../../../../images/web_GUI/icon/10_receive from actuator.png" style="position: relative;top: -100px;left: 8px; cursor:pointer;" id="load" onclick="GetACTStatus();">
           <img src="../../../../images/web_GUI/icon/10_send to actuator.png" style="position: relative;top: -100px;left: 505px; cursor:pointer;" id="save" onclick="SetACTStatus();">
           <img src="../../../../images/web_GUI/icon/10_actuator img.png" style="position: relative;top: -120px;left: -70px;">
           <img src="../../../../images/web_GUI/icon/10_MFD.png" style="position: relative; top: -245px;left: 255px;">
           <table class="actTable">
             <tr class="actTableTr">
               <td class="checkBox">
                 <input type="checkbox" id="checkForward" name="actCheck" class="cb" onclick="actuatorCheckbox('forward')" checked="checked">
                 <label for="checkForward"></label>
               </td>
               <td>
                 <img src="../../../../images/web_GUI/icon/acctuatorSetting.png" width="50px" height="30px" style="position:relative; top:3px; left:15px;">
               </td>
             </tr>
             <tr class="actTableTr">
               <td class="checkBox">
                 <input type="checkbox" id="checkReverse" name="actCheck" class="cb" onclick="actuatorCheckbox('reverse')">
                 <label for="checkReverse"></label>
               </td>
               <td>
                 <img src="../../../../images/web_GUI/icon/acctuatorRevers.png" width="50px" height="30px" style="position:relative; top:3px; left:15px;">
               </td>
             </tr>
           </table>
           <input type="hidden" id="direction_val" value="0">
           <input type="text" id="position_val" name="" value="0" style="width:90px; height:24px; background-color:transparent; border:none; text-align:center; color:white;position: relative;top: -325px;left: 136px;" disabled>
         </li>
         <li class="actuatorBox2" style="text-align:center; vertical">
           <div class="sliderContainer" style="position:relative; top:15px">
             <div id="Slider1" style="display:inline-block; margin-bottom:5px;"></div>
             <img src="../../../../images/web_GUI/icon/10_gauge img.png">
           </div>
         </li>
         <span style="color:white; font-size:12px; position:relative; top:-35px;left: 95px;">-50mm</span>
         <span style="color:white; font-size:12px; position:relative; top:-35px;left: 520px;">50mm</span>
       </ul>
       <table style="position:relative;top: 0;left: 80px;">
         <tr>
           <td style="color:white; font-size:15px; text-align:center;">Left Limit</td>
         </tr>
         <tr>
           <td><input class="table_textbox" style="width: 100px;" type="text" name="left_limit_val" id="left_limit_val" value=""></td>
         </tr>
       </table>
       <table style="position:relative;top: 10px;left: 315px;">
         <tr>
           <td style="color:white; font-size:15px; text-align:center;">ORIGIN</td>
         </tr>
         <tr>
           <td><input class="table_textbox" style="width: 100px;" type="text" name="origin_val" id="origin_val" value=""></td>
         </tr>
       </table>
       <table style="position:relative;top: -124px;left: 540px; z-index:5;">
         <tr>
           <td style="color:white; font-size:15px; text-align:center;">Right Limit</td>
         </tr>
         <tr>
           <td><input class="table_textbox" style="width: 100px;" type="text" name="right_limit_val" id="right_limit_val" value=""></td>
         </tr>
       </table>
       <div style="position:relative;top: -180px;left: 225px;">
         <img src="../../../../images/web_GUI/icon/10_left.png" style="margin-right:70px;  cursor:pointer;" id="left_move" onclick="Act_left_button(event);">
         <img src="../../../../images/web_GUI/icon/10_origin.png" style="cursor:pointer;" id="center_move" onclick="Act_origin_button(event);">
         <img src="../../../../images/web_GUI/icon/10_right.png" style="margin-left:70px;  cursor:pointer;" id="right_move" onclick="Act_right_button(event);">
       </div>
     </div>
     <script src="../../checkBoxControl.js"></script>
     <script src="../../../../splice.js"></script>
     <script>
      GetIPfromFile();
      GetACTStatus();
     </script>
   </body>
 </html>
