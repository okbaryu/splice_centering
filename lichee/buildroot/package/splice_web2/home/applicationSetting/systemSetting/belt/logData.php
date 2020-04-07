<?php
 ?>
 <!DOCTYPE html>
 <html>
   <head>
     <meta charset="utf-8">
    <link rel="stylesheet" type="text/css" href="/font.css">
     <link rel="stylesheet" type="text/css" href="/home/applicationSetting/systemSetting/belt/logData.css">
     <title></title>
   </head>
   <body>
     <div class="title">Real Time Data Log</div>
     <div class="log" style="padding:5px;">
       <textarea name="logData" id="message" rows="18" cols="72"style="resize:none; width: 625px; height: 353px; background-color:black; color:white; border:0;"></textarea>
     </div>
     <div class="saveButton button">
       <div class="icon"><img src="../../../../images/web_GUI/icon/15_save to sd card.png"></div>
       <div class="text">Save to <br>SD Card</div>
     </div>
     <div class="downloadButton button">
       <div class="icon"><img src="../../../../images/web_GUI/icon/15_download_to_pc.png"></div>
       <div class="text">Download <br>To PC</div>
     </div>
     <script src="../../../../splice.js"></script>
     <script>
        caliSelect('nomal');
      </script>
   </body>
 </html>
