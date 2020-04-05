<?php
 ?>
 <!DOCTYPE html>
 <html>
   <head>
     <meta charset="utf-8">
    <link rel="stylesheet" type="text/css" href="/font.css">
     <link rel="stylesheet" type="text/css" href="/home/applicationSetting/systemSetting/belt/systemOverview.css">
     <title></title>
   </head>
   <body>
     <div class="container">
       <ul>
         <li class="icon"><img src="../../../../images/web_GUI/icon/14_auto.png"></li>
         <li class="icon"><img src="../../../../images/web_GUI/icon/14_manual.gif"></li>
         <li class="icon"><img src="../../../../images/web_GUI/icon/14_origin.png"></li>
       </ul>
       <table class="table1">
         <tr>
           <td class="table_head">Last Belt</td>
         </tr>
         <tr>
           <td class="tag">Minimum Width</td>
           <td class="box minWidth td_text">0.00 mm</td>
         </tr>
         <tr>
           <td class="tag">Maximum Width</td>
           <td class="box maxWidth td_text">0.00 mm</td>
         </tr>
         <tr>
           <td class="tag">Average Width</td>
           <td class="box avgWidth td_text">0.00 mm</td>
         </tr>
         <tr>
           <td class="tag">Web Speed</td>
           <td class="box1 webSpeed td_text">0.00 m/min</td>
         </tr>
         <tr>
           <td class="table1_foot">Guiding OK</td>
         </tr>
       </table>
       <table class="table2">
         <tr>
           <td class="table_head">Input Signals</td>
         </tr>
         <tr>
           <td>
             <input type="checkbox" id="inputSig1" name="encoderCheck" class="cb" value="">
             <label for="inputSig1"></label>
             <span class="label_text">Roller is Down</span>
           </td>
           <td style="position:relative; left:-90px">
             <input type="checkbox" id="inputSig2" name="encoderCheck" class="cb" value="">
             <label for="inputSig2"></label>
             <span class="label_text">Stopped for Cutting</span>
           </td>
         </tr>
         <tr>
           <td>
             <input type="checkbox" id="inputSig3" name="encoderCheck" class="cb" value="">
             <label for="inputSig3"></label>
             <span class="label_text">No Title</span>
           </td>
           <td style="position:relative; left:-90px">
             <input type="checkbox" id="inputSig4" name="encoderCheck" class="cb" value="">
             <label for="inputSig4"></label>
             <span class="label_text">No Title</span>
           </td>
         </tr>
         <tr>
           <td>
             <input type="checkbox" id="inputSig5" name="encoderCheck" class="cb" value="">
             <label for="inputSig5"></label>
             <span class="label_text">Start Average Width Calculation</span>
           </td>
         </tr>
         <tr>
           <td>
             <input type="checkbox" id="inputSig6" name="encoderCheck" class="cb" value="">
             <label for="inputSig6"></label>
             <span class="label_text">No Title</span>
           </td>
           <td style="position:relative; left:-150px">
             <input type="checkbox" id="inputSig7" name="encoderCheck" class="cb" value="">
             <label for="inputSig7"></label>
             <span class="label_text">No Title</span>
           </td>
           <td style="position:relative; left:-200px">
             <input type="checkbox" id="inputSig8" name="encoderCheck" class="cb" value="">
             <label for="inputSig8"></label>
             <span class="label_text">No Title</span>
           </td>
         </tr>
       </table>
       <table class="table3">
         <tr>
           <td class="table_head">Output Signals</td>
         </tr>
         <tr>
           <td>
             <input type="checkbox" id="outputSig1" name="encoderCheck" class="cb" value="">
             <label for="outputSig1"></label>
             <span class="label_text">Stop Belt</span>
           </td>
           <td>
             <input type="checkbox" id="outputSig2" name="encoderCheck" class="cb" value="">
             <label for="outputSig2"></label>
             <span class="label_text">Run Belt</span>
           </td>
         </tr>
         <tr>
           <td>
             <input type="checkbox" id="outputSig3" name="encoderCheck" class="cb" value="">
             <label for="outputSig3"></label>
             <span class="label_text">Roller Down</span>
           </td>
           <td>
             <input type="checkbox" id="outputSig4" name="encoderCheck" class="cb" value="">
             <label for="outputSig4"></label>
             <span class="label_text">Splice Deteced</span>
           </td>
         </tr>
         <tr>
           <td>
             <input type="checkbox" id="outputSig5" name="encoderCheck" class="cb" value="">
             <label for="outputSig5"></label>
             <span class="label_text">No Title</span>
           </td>
           <td>
             <input type="checkbox" id="outputSig6" name="encoderCheck" class="cb" value="">
             <label for="outputSig6"></label>
             <span class="label_text">No Title</span>
           </td>
         </tr>
         <tr>
           <td>
             <input type="checkbox" id="outputSig7" name="encoderCheck" class="cb" value="">
             <label for="outputSig7"></label>
             <span class="label_text">No Title</span>
           </td>
           <td>
             <input type="checkbox" id="outputSig8" name="encoderCheck" class="cb" value="">
             <label for="outputSig8"></label>
             <span class="label_text">No Title</span>
           </td>
         </tr>
       </table>
     </div>
     <div class="backImg">
       <img src="../../../../images/web_GUI/web_GUI/14-system_belt-System overview.jpg" width="750px" height="265px">
       <table style="position:relative; top:-250px; left: 150px;">
         <tr>
           <td style="font-size: 12px;">Left Edge</td>
         </tr>
         <tr>
           <td width="102px" height="30px" style="background-color: #e03420; color:white; text-align:center; line-height:30px;">0.0 mm</td>
         </tr>
       </table>
       <table style="position:relative; top:-308px; left: 450px;">
         <tr>
           <td style="font-size: 12px;">Current Width</td>
         </tr>
         <tr>
           <td width="102px" height="30px" style="background-color: #e03420; color:white; text-align:center; line-height:30px;">0.0 mm</td>
         </tr>
       </table>
       <table style="position:relative; top:-366px; left: 600px;">
         <tr>
           <td style="font-size: 12px;">Right Edge</td>
         </tr>
         <tr>
           <td width="102px" height="30px" style="background-color: #e03420; color:white; text-align:center; line-height:30px;">0.0 mm</td>
         </tr>
       </table>
       <table style="position:relative; top:-219px; left: 500px;">
         <tr>
           <td>Motor Pos.</td>
           <td width="102px" height="30px" style="background-color: #353535; color:white; text-align:center; line-height:30px;">0.0 mm</td>
         </tr>
       </table>
       <div class="" style="position:relative; top:-256px; left:30px; width: 90px; height: 35px; background-color: #131315;">
         <img src="../../../../images/web_GUI/icon/14_motor.png">
       </div>
     </div>
     <script src="../../../../splice.js"></script>
     <script>
       caliSelect('nomal');
     </script>
   </body>
 </html>
