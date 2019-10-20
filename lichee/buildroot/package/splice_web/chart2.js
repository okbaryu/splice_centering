/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
// SVG-VML-3D 1.3 by Lutz Tautenhahn 2002-2006
// The Author grants you a non-exclusive, royalty free, license to use,
// modify and redistribute this software.
// This software is provided "as is", without a warranty of any kind.
if (1) {
    document.writeln("<table border=0 style='border:1px solid white;' ><tr><th valign=top align=left >");
    // the embed width here should be at least twice the width of the <svg> elements
    document.writeln( "<embed width='1440' height='10' name='Scene1' wmode='transparent' type='image/svg+xml' border=0 />");
    document.writeln( "<table cols=2 ><tr>");
    document.writeln( "<td align=left><svg width='650' height='484' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' onload='OnLoadEvent222(evt)' ");
    document.writeln( "id=svg222 style='border:1px solid white;' ><g id='Scene222'><text x=50 y=50>Scene222</text></g></svg></td>");
    document.writeln( "<td align=left><svg width='650' height='484' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' onload='OnLoadEvent333(evt)' ");
    document.writeln( "id=svg333 style='border:1px solid white;' ><g id='Scene333'><text x=50 y=50>Scene333</text></g></svg></td>");
    document.writeln( "</tr></table>");
    document.writeln( "</embed>");

    document.writeln("</th></tr></table>");

    document.writeln("<div id=SVG_DATA_ARRAY >");
    document.writeln("<table border=0 cellpadding=0 cellspacing=0>");
    document.write("<tr><th>Z</th>");
    for(j=0; j<Ymax; j++) {
        document.write("<th>Y"+j+"</th>");
    }
    document.writeln("<th>Scale</th><th>Color</th></tr>");
    for (i=0; i<Xmax; i++)
    {    document.write("<tr><th>X"+i+"</th>");
         for(j=0; j<Ymax; j++) {
             document.write("<th><input class=svg name='v"+i+"_"+j+"' value='"+Math.floor(80-10*i-10*Math.random())+"'></th>");
         }
         document.writeln("<th><input class=svg name='s_x"+i+"' value='"+eval(i+1)+"'></th><th><input class=svg name='c_x"+i+"' value='"+GetCol(i)+"'></th></tr>");
    }
    document.write("<tr><th>Scale</th>");
    for(j=0; j<Ymax; j++) {
        document.write("<th><input class=svg name='s_y"+j+"' value='"+String.fromCharCode(65+j)+"'></th>");
    }
    document.writeln("<th><input type='button' onClick='ClearGrid()' style='width:44' value='Clear' /></th><th><input type='button' onClick='DrawScene()' style='width:44' value='Draw' /></th></tr>");
    document.writeln("</table>");
    document.writeln("</div >");
}

function GetCol(ii)
{ if (ii==0) return("#8080ff");
  if (ii==1) return("#ff8080");
  if (ii==2) return("#80ff80");
  if (ii==3) return("#ff80ff");
  if (ii==4) return("#ffff80");
  return("#ffffff");
}

function ClearGrid()
{ var ii, jj, obj, ii2;
  var lvalue='v';
  for (ii=0; ii<Xmax; ii++)
  {
    if(ii==8) { // if we are halfway through the array, switch to the lable for the 2nd half of the array
        lvalue='V';
    }
    ii2=ii;
    if(ii>=8) { // if we are halfway through the array, switch to the lable for the 2nd half of the array
        ii2=ii-8;
    }
    //alert("ClearGrid: lvalue=(" + lvalue + ")   ii2=" + ii2 );
    for (jj=0; jj<Ymax; jj++)
    {
      obj = document.forms[0][lvalue+ii2+'_'+jj];
      if(obj) obj.value=""; else break; // CAD added break
    }
  }
  return true;
}

function RowIsHidden(value, ii)
{
    var rowid1 = "";
    var rowobj1 = 0;
    if (value == 'v') {
        rowid1 = "r_x"+ii;
    } else {
        rowid1 = "R_x"+ii;
    }
    rowobj1 = document.getElementById( rowid1 );
    if (rowobj1 && rowobj1.getAttribute('style').indexOf("hidden") > 1 ) { // if the row is known and it is not hidden
        return true;
    } else {
        return false;
    }
}

function DrawScene(which)
{ var ii, jj, cc, vv, xmin=-1, xmax=0, ymin=-1, ymax=0, zmin=0, zmax=0;
  var obj;
  var debug=0;
  if (which == null) {
      if ( GetRxTxRadioButtonSelected() == "RX") {
          which=1;
      } else {
          which=0;
      }
  }
  sidx = which;
  if(debug==1) alert("DrawScene: sidx=" + sidx );
  if(debug) alert("S:" + S + "; B:" + B + "; this.idx:" + this.idx + "; SVGObjects.len:" + SVGObjects.length + "; sidx:" + sidx );

  for (ii=start_idx; ii<Xmax; ii++)
  {
    if ( RowIsHidden(value,ii) == false ) {
        for (jj=0; jj<Ymax; jj++)
        {
          obj=document.forms[0][value+ii+'_'+jj];
          if(debug) if(ii==start_idx && jj==8) alert("obj (" + value + ii+"_"+jj+"):" + obj + "; which:" + which );
          if (obj) {
              vv=obj.value;
              //alert("for v" + ii + "_" + jj + " ... = " + vv + "xmax:" + xmax + "; ymax:" + ymax + "; zmax:" + zmax + "; Xmax:" + Xmax + "; Ymax:" + Ymax );
              if (vv!="")
              { vv=parseFloat(vv);
                if (xmax<ii) xmax=ii;
                if (ymax<jj) ymax=jj;
                if (zmin>vv) zmin=vv;
                if (zmax<vv) zmax=vv;
              }
          } else {
            //continue;
          }
        }
    } else {
        if(debug) alert("DrawScene: rowid (" + rowid1 + ") ... is hidden; skipping it");
    }
  }
  if(debug) alert("xmax:" + xmax +";   ymax:" + ymax +";   zmax:" + zmax );
  xmax++;
  ymax++;
  if (zmin==zmax)
  { var SceneIds = [ "Scene222","Scene333" ];
    var obj= document.getElementById( SceneIds[which] );
    if ( obj ) obj.innerHTML = "<text x=50 y=50>No data to graph!</text>";
    return;
  }
  if ( S[sidx].Delete )
  {
      if(debug) alert("S[sidx].Delete" );
      S[sidx].Delete();
  }
  if(debug) alert("S[sidx].Init");
  S[sidx].Init();
  //either use S[sidx].Init() to keep the rotation and zoom settings or use the following 2 lines to make all new
  //if (useSVG) S=new Scene3D(SVGObjects[0],0,500,500);
  //else S=new Scene3D(document.getElementById("Scene1"),1);
  if (S[sidx].Zoom.y!=1)
  { S[sidx].Zoom.y=1;
    S[sidx].YM-=30;
  }
  S[sidx].Zoom.z=8/(zmax-zmin);
  if(debug) alert("DrawScene 1");
  B[sidx]=new BoundingBox(S[sidx], "#ffffff", "#000000");
  if(debug) alert("new BoundingBox");
  for (ii=start_idx; ii<Xmax; ii++)
  { obj=document.forms[0][color+'_x'+ii];
    if(obj && RowIsHidden(value,ii) == false) {
        cc= obj.value;
        //if(debug) alert("color obj (" + value + ii + "):" + obj + "; which:" + which );
        if (cc=="") cc="#cccccc";
        for (jj=0; jj<Ymax; jj++)
        { obj=document.forms[0][value+ii+'_'+jj];
          if(debug) if(ii==start_idx && jj==0) alert("obj (" + value + ii+"_"+jj+"):" + obj + "; which:" + which );
          if(obj) {
              vv=obj.value;
              if (vv!="")
              { vv=parseFloat(vv);
                if (vv>0) bb=new Box3D(S[sidx], ii-0.3, jj-0.3, 0, ii+0.3, jj+0.3, vv, cc, cc, "#000000", 1);
                if (vv<0) bb=new Box3D(S[sidx], ii-0.3, jj-0.3, vv, ii+0.3, jj+0.3, 0, cc, cc, "#000000", 1);
                bb.SetId(which+"_"+ii+"_"+jj);
                bb.SetEventAction("mouseover",parent.MouseOver);
                bb.SetEventAction("mouseout",parent.MouseOut);
              }
          } else  {
              if(debug) alert("color ... skipping rest of jj ... " + jj );
              break; // CAD added break
          }
        }
    } else {
        //continue; // CAD added break
    }
  }
  S[sidx].AutoCenter();
  S[sidx].Dist*=1.8;
  B[sidx].SetBorder(xmin,ymin,zmin,xmax,ymax,zmax);
  B[sidx].GridDelta.x=1;
  B[sidx].GridDelta.y=1;
  B[sidx].Scale.x="function MyXScale";
  B[sidx].Label.x="";
  B[sidx].Scale.y="function MyYScale";
  B[sidx].Scale.y=1; // if 1 => show label; if < 1 => nothing; if > 1 => date
  B[sidx].Label.y="";
  B[sidx].Label.z="";
  if(debug) alert("DrawScene 2");
  S[sidx].OrderWeight.z=0.05;
  S[sidx].Sort();
  if(debug) alert("DrawScene 3");
  //if(debug) alert("S[sidx].Draw() ... (" + S[sidx].Draw + ")" );
  S[sidx].Draw();
  if(debug) alert("DrawScene 4");
  viewerzoomed=0;
  picturezoomed=0;
  if(debug) alert("DrawScene(" + which + ") done");
  return true;
}
function MouseOver(evt)
{ var ii, cc="#ff8080", obj;
  // the event id is something like: 0_8_11 where the first 0 indicates SVG[0] or SVG[1]
  if (evt)
  {ii=evt.target.id; //SVG
   cc=evt.target.getAttributeNS(null,"fill");
  }
  else
  { ii=this.id; //VML
    cc=""+this.fillcolor;
    //or cc=""+event.srcElement.fillcolor;
  }
  //alert("MouseOver: id=(" + ii + ");  substr(" + ii.substr(2,32) );
  if (ii.charAt(0) == '0') {
      obj=document.forms[0]['v'+ii.substr(2,32)];
  } else {
      obj=document.forms[0]['V'+ii.substr(2,32)];
  }
  if(obj) obj.style.backgroundColor=cc;
  return true;
}
function MouseOut(evt)
{ var ii, obj;
  // the event id is something like: 0_8_11 where the first 0 indicates SVG[0] or SVG[1]
  if (evt) ii=evt.target.id; //SVG
  else ii=this.id; //VML
  if (ii.charAt(0) == '0') {
      obj=document.forms[0]['v'+ii.substr(2,32)];
  } else {
      obj=document.forms[0]['V'+ii.substr(2,32)];
  }
  if(obj) obj.style.backgroundColor="#ffffff";
  return true;
}
function MyXScale(xx)
{ var obj=document.getElementById(scale+'_x'+xx);
  //if(xx==13) alert("MyXScale(" + xx + ")  Xmax:" + Xmax );
  if ((xx>=start_idx)&&(xx<Xmax)&&obj) return( obj.innerHTML );
  return("");
}
function MyYScale(yy)
{ var obj=document.getElementById(scale+'_y'+yy);
  //alert("MyYScale(" + yy + ")" );
  if ((yy>=0)&&(yy<Ymax)&&obj) return( obj.innerHTML );
  return("");
}
