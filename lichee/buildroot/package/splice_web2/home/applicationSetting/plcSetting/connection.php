<?php
 ?>
 <!DOCTYPE html>
 <html>
   <head>
     <meta charset="utf-8">
     <link rel="stylesheet" href="./connection.css">
     <title></title>
   </head>
   <body>
     <div class="container">
       <ul>
         <li class="ethernet">
           <div style="position:relative; top:30px; left:30px;">
             <input type="checkbox" id="ethernetUse" name="protocol" class="cb" onclick="protocolSelect('ethernetUse')" value="">
             <label for="ethernetUse"></label>
             <span>Ethernet</span>
             <table style="position:relative; left:30px;">
               <tr>
                 <td>IP-Address</td>
                 <td><input type="text" name="" value=""></td>
               </tr>
               <tr>
                 <td>Dest. Port</td>
                 <td><input type="text" name="" value=""></td>
               </tr>
             </table>
             <div style="position:relative; top:-80px; left:400px; width:50px;">
               <span>Protocol</span>
               <select class="protoSelect" style="border:none; color:white;" name="">
                 <option style="color: white; background:#2f2f2f" value="not_select">Not Selected</option>
                 <option style="color: white; background:#111111" value="sicmens_s7_udp">Sicmens S7 udp</option>
                 <option style="color: white; background:#2f2f2f" value="beckhoff">Beckhoff</option>
                 <option style="color: white; background:#111111" value="mitsubishi">Mitsubishi</option>
                 <option style="color: white; background:#2f2f2f" value="modbus_tcp">Modbus TCP</option>
               </select>
             </div>
           </div>
         </li>
         <li class="serial">
           <table style="border-collapse: separate; border-spacing:70px; position:relative; top:-39px; left: -39px;">
             <tr>
               <td>
                 <input type="checkbox" id="422Use" name="protocol" class="cb" onclick="protocolSelect('422Use')" value="">
                 <label for="422Use"></label>
                 <span>RS-422</span>
               </td>
               <td>
                 <input type="checkbox" id="232Use" name="protocol" class="cb" onclick="protocolSelect('232Use')" value="">
                 <label for="232Use"></label>
                 <span>RS-232</span>
               </td>
             </tr>
           </table>
           <div style="position:relative; top:-100px; left: 22px;">
             <ul style="list-style:none; width:240px; height:25px;">
               <li style="width: 160px; height:25px; float:left; color:white;">Baud Rate</li>
               <li style="width: 80px; height:25px; float:left;">
                 <select class="serialSelect" name="">
                   <option style="color: white; background:#111111" value="9600">9600</option>
                   <option style="color: white; background:#111111" value="19200">19200</option>
                   <option style="color: white; background:#111111" value="38400">38400</option>
                   <option style="color: white; background:#111111" value="57600">57600</option>
                   <option style="color: white; background:#111111" value="115200">115200</option>
                   <option style="color: white; background:#111111" value="230400">230400</option>
                 </select>
               </li>
             </ul>
             <ul style="list-style:none; width:240px; height:25px;">
               <li style="width: 160px; height:25px; float:left; color:white;">Parity</li>
               <li style="width: 80px; height:25px; float:left;">
                 <select class="serialSelect" name="">
                   <option style="color: white; background:#111111" value="none">None</option>
                   <option style="color: white; background:#111111" value="odd">Odd</option>
                   <option style="color: white; background:#111111" value="even">Even</option>
                 </select>
               </li>
             </ul>
             <ul style="list-style:none; width:240px; height:25px;">
               <li style="width: 160px; height:25px; float:left; color:white;">Parity Error Char.</li>
               <li style="width: 80px; height:25px; float:left;">
                 <select class="serialSelect" name="">
                   <option style="color: white; background:#111111" value="63">63(?)</option>
                 </select>
               </li>
             </ul>
           </div>
         <div style="position: relative; top:-187px; left:260px;">
             <ul style="list-style:none; width:200px; height:25px;">
               <li style="width: 120px; height:25px; float:left; color:white;">Data Bits</li>
               <li style="width: 80px; height:25px; float:left;">
                 <select class="serialSelect" name="">
                   <option style="color: white; background:#111111" value="5">5</option>
                   <option style="color: white; background:#111111" value="6">6</option>
                   <option style="color: white; background:#111111" value="7">7</option>
                   <option style="color: white; background:#111111" value="8" selected>8</option>
                 </select>
               </li>
             </ul>
             <ul style="list-style:none; width:200px; height:25px;">
               <li style="width: 120px; height:25px; float:left; color:white;">Stop Bits</li>
               <li style="width: 80px; height:25px; float:left;">
                 <select class="serialSelect" name="">
                   <option style="color: white; background:#111111" value="1">1</option>
                   <option style="color: white; background:#111111" value="1.5">1.5</option>
                   <option style="color: white; background:#111111" value="2">2</option>
                 </select>
               </li>
             </ul>
           </div>
         </li>
         <li class="dataLenght">
           <span style="position:relative; top:20px; left:-260px;">Data Length</span>
           <table style="position:relative; top:20px; left:-250px;">
             <tr>
               <td style="font-size:14px;">Read Start Address</td>
               <td><input type="text" name="" value=""></td>
             </tr>
             <tr>
               <td style="font-size:14px;">Write Start Address</td>
               <td><input type="text" name="" value=""></td>
             </tr>
           </table>
           <table style="position:relative; top:-38px; left:140px;">
             <tr>
               <td>Length</td>
               <td><input type="text" name="" value=""></td>
             </tr>
             <tr>
               <td>Length</td>
               <td><input type="text" name="" value=""></td>
             </tr>
           </table>
         </li>
       </ul>
     </div>
     <script src="../checkBoxControl.js"></script>
   </body>
 </html>
