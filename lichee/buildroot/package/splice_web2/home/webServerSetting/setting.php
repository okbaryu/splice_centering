<?php
 ?>
 <!DOCTYPE html>
 <html>
   <head>
     <meta charset="utf-8">
     <link rel="stylesheet" href="./setting.css">
     <title></title>
   </head>
   <body>
      <div class="box" style="position:relative; top:60px; left:30px;">
         <div class="box_label">Controller</div>
         <table>
           <tr>
             <td class="table_label">IP</td>
             <td class="table_value">
               <input type="text" name="" value="10.6.33.110">
             </td>
           </tr>
           <tr>
             <td class="table_label">Subnet mask</td>
             <td class="table_value">
               <input type="text" name="" value="255.255.0.0">
             </td>
           </tr>
           <tr>
             <td class="table_label">Sefault gateway</td>
             <td class="table_value">
               <input type="text" name="" value="0.0.0.0">
             </td>
           </tr>
           <tr>
             <td class="table_label">DNS Server</td>
             <td class="table_value">
               <input type="text" name="" value="127.0.0.1">
             </td>
           </tr>
         </table>
      </div>
      <div class="box" style="position:relative; top:-120px; left:370px;">
          <div class="box_label">Actuator</div>
          <table>
            <tr>
              <td class="table_label">IP</td>
              <td class="table_value">
                <input type="text" name="" value="10.6.33.110">
              </td>
            </tr>
            <tr>
              <td class="table_label">Subnet mask</td>
              <td class="table_value">
                <input type="text" name="" value="255.255.0.0">
              </td>
            </tr>
            <tr>
              <td class="table_label">Sefault gateway</td>
              <td class="table_value">
                <input type="text" name="" value="0.0.0.0">
              </td>
            </tr>
            <tr>
              <td class="table_label">DNS Server</td>
              <td class="table_value">
                <input type="text" name="" value="127.0.0.1">
              </td>
            </tr>
          </table>
      </div>
      <div class="system">
          <div class="box_label">System Time</div>
          <div class="cal_box">
            <span style="margin-left:10px;">날짜</span>
            <div class="calendar">

            </div>
          </div>
          <div class="clock_box">
            <span>시간</span>
            <table style="position: relative; left:50px;">
              <tr>
                <td>현재 시간 : </td>
                <td id="dateTimeNow"></td>
              </tr>
            </table>
            <table style="border-collapse: separate; border-spacing: 5px; position:relative; left:80px;">
              <tr>
                <td class="table_label">시</td>
                <td class="table2_value">
                  <input type="text" name="" value=""></td>
              </tr>
              <tr>
                <td class="table_label">분</td>
                <td class="table2_value">
                  <input type="text" name="" value=""></td>
              </tr>
              <tr>
                <td class="table_label">초</td>
                <td class="table2_value">
                  <input type="text" name="" value=""></td>
              </tr>
            </table>
          </div>
          <input type="submit" id="applyButton" name="apply" value="Apply">
      </div>
      <script type="text/javascript">
        var timeTarget = document.getElementById("dateTimeNow");

        function clock() {
          var date = new Date();
          var hours = date.getHours();
          var minutes = date.getMinutes();
          var seconds = date.getSeconds();
          timeTarget.innerText = `${hours < 10 ? `0${hours}` : hours}:${minutes < 10 ? `0${minutes }`  : minutes }:${seconds < 10 ? `0${seconds }`  : seconds }`;
        }

        function init() {
        clock();
        setInterval(clock, 1000);
        }

        init();
      </script>
   </body>
 </html>
