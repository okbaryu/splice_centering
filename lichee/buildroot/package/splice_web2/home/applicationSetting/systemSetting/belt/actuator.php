<?php
 ?>
 <!DOCTYPE html>
 <html>
   <head>
     <meta charset="utf-8">
     <link rel="stylesheet" href="./actuator.css">
     <title></title>
   </head>
   <body>
     <div class="container">
       <table style="position:relative;left: 270px;top: 20px;">
         <tr>
           <td style="color:white; font-size:15px; text-align:center;">IP ADDRESS TO CONNECT</td>
         </tr>
         <tr>
           <td><input class="table_textbox" style="width: 180px;" type="text" name="" value="192.168.0.170"></td>
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
               <td><input class="table_textbox" style="width: 100px;" type="text" name="" value=""></td>
             </tr>
           </table>
           <table style="position:relative; top: -57px;left: 305px;">
             <tr>
               <td style="color:white; font-size:15px; text-align:center;">SPEED</td>
             </tr>
             <tr>
               <td><input class="table_textbox" style="width: 100px;" type="text" name="" value=""></td>
             </tr>
           </table>
           <img src="../../../../images/web_GUI/icon/10_receive from actuator.png" style="position: relative;top: -100px;left: 8px;">
           <img src="../../../../images/web_GUI/icon/10_send to actuator.png" style="position: relative;top: -100px;left: 505px;">
           <img src="../../../../images/web_GUI/icon/10_actuator img.png" style="position: relative;top: -120px;left: -70px;">
           <img src="../../../../images/web_GUI/icon/10_MFD.png" style="position: relative; top: -185px;left: -400px;">
           <table class="actTable">
             <tr class="actTableTr">
               <td class="checkBox">
                 <input type="checkbox" id="checkForward" name="actCheck" class="cb" onclick="actuatorCheckbox('forward')" value="forward">
                 <label for="checkForward"></label>
               </td>
               <td>
                 <img src="../../../../images/web_GUI/icon/acctuatorSetting.png" width="50px" height="30px" style="position:relative; top:3px; left:15px;">
               </td>
             </tr>
             <tr class="actTableTr">
               <td class="checkBox">
                 <input type="checkbox" id="checkReverse" name="actCheck" class="cb" onclick="actuatorCheckbox('reverse')" value="reverse">
                 <label for="checkReverse"></label>
               </td>
               <td>
                 <img src="../../../../images/web_GUI/icon/acctuatorRevers.png" width="50px" height="30px" style="position:relative; top:3px; left:15px;">
               </td>
             </tr>
           </table>
           <input type="text" name="" value="0.0 mm" style="width:90px; height:24px; background-color:#e03420; border:none; text-align:center; color:white;position: relative;top: -262px;left: 136px;">
         </li>
         <li class="actuatorBox2" style="text-align:center; vertical">
           <div class="slidecontainer" style="display:inline; position:relative; top:10px;">
             <input type="range" min="-50" max="50" value="0" class="slider" id="myRange">
             <img src="../../../../images/web_GUI/icon/10_gauge img.png" style="margin-top:5px;">
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
           <td><input class="table_textbox" style="width: 100px;" type="text" name="" value=""></td>
         </tr>
       </table>
       <table style="position:relative;top: 10px;left: 315px;">
         <tr>
           <td style="color:white; font-size:15px; text-align:center;">ORIGIN</td>
         </tr>
         <tr>
           <td><input class="table_textbox" style="width: 100px;" type="text" name="" value=""></td>
         </tr>
       </table>
       <table style="position:relative;top: -124px;left: 540px;">
         <tr>
           <td style="color:white; font-size:15px; text-align:center;">Right Limit</td>
         </tr>
         <tr>
           <td><input class="table_textbox" style="width: 100px;" type="text" name="" value=""></td>
         </tr>
       </table>
       <div style="position:relative;top: -180px;left: 225px;">
         <img src="../../../../images/web_GUI/icon/10_left.png" style="margin-right:70px;">
         <img src="../../../../images/web_GUI/icon/10_origin.png">
         <img src="../../../../images/web_GUI/icon/10_right.png" style="margin-left:70px;">
       </div>
     </div>
     <script src="../../checkBoxControl.js"></script>
   </body>
 </html>
