<?php
 ?>
 <!DOCTYPE html>
 <html>
   <head>
     <meta charset="utf-8">
    <link rel="stylesheet" type="text/css" href="/font.css">
     <link rel="stylesheet" type="text/css" href="/home/applicationSetting/systemSetting/belt/encoder.css">
     <title></title>
   </head>
   <body>
     <table class="table1">
       <tr>
         <td>
           <input type="checkbox" id="checkUse" name="encoderCheck" class="cb" onclick="encoderCheckbox('use')" value="use" checked="checked">
           <label for="checkUse"></label>
         </td>
         <td class="checkBox_lable">Use</td>
       </tr>
       <tr>
         <td>
           <input type="checkbox" id="checkUnuse" name="encoderCheck" class="cb" onclick="encoderCheckbox('unuse')" value="unuse">
           <label for="checkUnuse"></label>
         </td>
         <td class="checkBox_lable">Unuse</td>
       </tr>
     </table>
     <div class="box">
         <div class="box_label">Encoder Specification</div>
       <table class="box_table">
         <tr>
           <td><input type="text" class="table_input" name="" value="2048"></td>
           <td>Pulse/Rev</td>
         </tr>
         <tr>
           <td><input type="text" class="table_input" name="" value="106.23"></td>
           <td>mm/Pulse</td>
         </tr>
       </table>
     </div>
     <input type="submit" id="applyButton" name="encoderApply" value="Apply">
     <script src="../../checkBoxControl.js"></script>
     <script src="../../../../splice.js"></script>
     <script>
       caliSelect('nomal');
     </script>
   </body>
 </html>
