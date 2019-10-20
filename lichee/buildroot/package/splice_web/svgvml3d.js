// SVG-VML-3D 1.3 by Lutz Tautenhahn 2002-2006
// The Author grants you a non-exclusive, royalty free, license to use,
// modify and redistribute this software.
// This software is provided "as is", without a warranty of any kind.
var useSVG=true;
if (navigator.appName == "Microsoft Internet Explorer") useSVG=false;
if (navigator.userAgent.search("Opera")>=0) useSVG=true;
SVGObjects=new Array();
function SVGObject(ii,oo,id)
{ this.idx=ii;
  //alert("SVGObject: ii=" + ii );
  this.GetSVGDoc=_GetSVGDoc;
  this.GetScene=_GetScene;
  this.SVGDocument=oo;
  this.ID=id;
  return true;
}
function AddSVGObject(oo,id)
{ var ll=SVGObjects.length;
  SVGObjects[ll]=new SVGObject(ll,oo,id);
  //alert("AddSVGObject: " + id + "; ll=" + ll );
  return true;
}
function _GetSVGDoc()
{ return(this.SVGDocument);
  //return(document.embeds['Scene1'].getSVGDocument());
}
function _GetScene()
{
    if ( this.ID == "svg333") {
        return(this.SVGDocument.getElementById("Scene222"));
    } else {
        return(this.SVGDocument.getElementById("Scene333"));
    }
  //return(document.embeds[this.idx].getSVGDocument().getElementById("Scene"));
}

function Vector(xx, yy, zz)
{ this.x=xx;
  this.y=yy;
  this.z=zz;
  this.Add=_VectorAdd;
  this.Zoom=_VectorZoom;
  this.Normalize=_VectorNormalize;
  return true;
}
function _VectorAdd(vv)
{ this.x+=vv.x;
  this.y+=vv.y;
  this.z+=vv.z;
  return true;
}
function _VectorZoom(ff)
{ this.x*=ff;
  this.y*=ff;
  this.z*=ff;
  return true;
}
function _VectorNormalize()
{ var ll=0;
  ll+=this.x*this.x;
  ll+=this.y*this.y;
  ll+=this.z*this.z;
  if (ll>0.0)
  { ll=Math.sqrt(ll);
    this.x/=ll;
    this.y/=ll;
    this.z/=ll;
  }
  else this.x=1.0;
  return true;
}

function Scene3D(aParentObj, azIndex, aWidth, aHeight)
{ this.Parent=aParentObj;
  this.Document=aParentObj.document;
  this.BoundingBox=null;
  this.Poly=new Array();
  this.PolyRank=new Array();
  this.Shape=new Array();
  this.zIndex=azIndex;
  this.Center=new Vector(0.0, 0.0, 0.0);
  this.Zoom=new Vector(1.0, 1.0, 1.0);
  this.OrderWeight=new Vector(1.0, 1.0, 1.0);
  this.ZoomAll=1.2;
  this.ShiftX=40.0;
  this.ShiftY=40.0;
  if (useSVG)
  { this.XM=aWidth/2;
    this.YM=aHeight/2;
  }
  else
  { this.XM=parseInt(aParentObj.style.width)/2;
    this.YM=parseInt(aParentObj.style.height)/2;
  }
  this.Dist=1000.0;
  this.Viewer=new Vector(1000.0, 0.0, 0.0);
  this.Th=0.0; this.Fi=0.0;
  this.DiffuseLight=0.5;
  this.Light=new Vector(1.0, 0.0, 0.0);
  this.ThLight=0.0; this.FiLight=0.0;
  this.sin_Th=0.0; this.cos_Th=1.0; this.sin_Fi=0.0; this.cos_Fi=1.0;
  this.cos_Fi_sin_Th=0.0; this.sin_Fi_sin_Th=0.0; this.cos_Fi_cos_Th=1.0; this.sin_Fi_cos_Th=0.0;
  this.LightTh=0.0;
  this.LightFi=0.0;
  this.ChangeViewer=_Scene3DChangeViewer;
  this.ChangeLight=_Scene3DChangeLight;
  this.AddPoly=_Scene3DAddPoly;
  this.AutoCenter=_Scene3DAutoCenter;
  this.ScreenPos=_Scene3DScreenPos;
  this.Sort=_Scene3DSort;
  this.Draw=_Scene3DDraw;
  this.Delete=_Scene3DDelete;
  this.ZoomUpdate=_Scene3DZoomUpdate;
  this.GetColor=_Scene3DGetColor;
  this.Init=_Scene3DInit;
  this.Callback=new Array();
  this.Init();
  return(this);
}
function _Scene3DInit()
{ if (useSVG)
  { this.BoxGroup=this.Parent.GetSVGDoc().createElementNS("http://www.w3.org/2000/svg","g");
    this.BoxGroup.setAttribute("x","0");
    this.BoxGroup.setAttribute("y","0");
    this.BoxGroup.setAttribute("width",this.XM*2);
    this.BoxGroup.setAttribute("height",this.YM*2);
    this.Parent.GetScene().appendChild(this.BoxGroup);
    this.PolyGroup=this.Parent.GetSVGDoc().createElementNS("http://www.w3.org/2000/svg","g");
    this.PolyGroup.setAttribute("x","0");
    this.PolyGroup.setAttribute("y","0");
    this.PolyGroup.setAttribute("width",this.XM*2);
    this.PolyGroup.setAttribute("height",this.YM*2);
    this.Parent.GetScene().appendChild(this.PolyGroup);
  }
  else
  { this.BoxGroup=this.Document.createElement("v:group");
    with (this.BoxGroup.style)
    { position="absolute"; top=0; left=0; width=parseInt(this.XM*2); height=parseInt(this.YM*2);
    }
    this.BoxGroup.coordorigin="0,0";
    this.BoxGroup.coordsize=parseInt(this.XM*2)+","+parseInt(this.YM*2);
    this.Parent.insertBefore(this.BoxGroup,null);
    this.PolyGroup=this.Document.createElement("v:group");
    with (this.PolyGroup.style)
    { position="absolute"; top=0; left=0; width=parseInt(this.XM*2); height=parseInt(this.YM*2);
    }
    this.PolyGroup.coordorigin="0,0";
    this.PolyGroup.coordsize=parseInt(this.XM*2)+","+parseInt(this.YM*2);
    this.Parent.insertBefore(this.PolyGroup,null);
  }
  return true;
}
function _Scene3DChangeViewer(ddTh, ddFi)
{ var pi_d_180=3.14159265/180;
  if ((this.Th+ddTh>=-89.0)&&(this.Th+ddTh<=89.0)) this.Th+=ddTh;
  this.Fi+=ddFi;
  while (this.Fi<0.0) this.Fi+=360.0;
  while (this.Fi>=360.0) this.Fi-=360.0;
  this.sin_Th=Math.sin(this.Th*pi_d_180);
  this.cos_Th=Math.cos(this.Th*pi_d_180);
  this.sin_Fi=Math.sin(this.Fi*pi_d_180);
  this.cos_Fi=Math.cos(this.Fi*pi_d_180);
  this.cos_Fi_sin_Th=this.cos_Fi*this.sin_Th;
  this.sin_Fi_sin_Th=this.sin_Fi*this.sin_Th;
  this.cos_Fi_cos_Th=this.cos_Fi*this.cos_Th;
  this.sin_Fi_cos_Th=this.sin_Fi*this.cos_Th;
  this.Viewer.x=this.Center.x+this.cos_Fi_cos_Th*this.Dist;
  this.Viewer.y=this.Center.y-this.sin_Fi_cos_Th*this.Dist;
  this.Viewer.z=this.Center.z-this.sin_Th*this.Dist;
  return true;
}
function _Scene3DChangeLight(ddTh, ddFi)
{ var pi_d_180=3.14159265/180;
  if ((this.ThLight+ddTh>=-89.0)&&(this.ThLight+ddTh<=89.0)) this.ThLight+=ddTh;
  this.FiLight+=ddFi;
  if (this.ThLight<-89.0) this.ThLight=-89.0;
  if (this.ThLight>89.0) this.ThLight=89.0;
  while (this.FiLight<0.0) this.FiLight+=360.0;
  while (this.FiLight>=360.0) this.FiLight-=360.0;
  this.Light.x=Math.cos(this.FiLight*pi_d_180)*Math.cos(this.ThLight*pi_d_180);
  this.Light.y=-Math.sin(this.FiLight*pi_d_180)*Math.cos(this.ThLight*pi_d_180);
  this.Light.z=-Math.sin(this.ThLight*pi_d_180);
  return true;
}
function _Scene3DAddPoly(oo)
{ var ii=this.Poly.length;
  this.Poly[ii]=oo;
  this.PolyRank[ii]=new Array(ii, 0);
  if (useSVG)
  { this.Shape[ii]=this.Parent.GetSVGDoc().createElementNS("http://www.w3.org/2000/svg","path");
    this.Shape[ii].setAttribute("z-index",this.zIndex+ii+3);
    this.Parent.GetScene().appendChild(this.Shape[ii]);
  }
  else
  { this.Shape[ii] = this.Document.createElement("v:shape");
    with (this.Shape[ii].style)
    { position="absolute"; left=0; top=0;
      width=this.PolyGroup.style.width;
      height=this.PolyGroup.style.height;
      zIndex=this.zIndex+ii+3; //reserve 0..2 for bounding box
    }
    this.PolyGroup.insertBefore(this.Shape[ii], null);
  }
  return true;
}
function _Scene3DAutoCenter()
{ var ii, jj, vv, xxmin, xxmax, yymin, yymax, zzmin, zzmax;
  var ll=this.Poly.length;
  this.Center.Zoom(0.0);
  for (ii=0; ii<ll; ii++)
    this.Center.Add(this.Poly[ii].Center);
  if (ll>0) this.Center.Zoom(1.0/ll);
  xxmin=this.Center.x;
  xxmax=this.Center.x;
  yymin=this.Center.y;
  yymax=this.Center.y;
  zzmin=this.Center.z;
  zzmax=this.Center.z;
  for (ii=0; ii<ll; ii++)
  { for (jj=0; jj<this.Poly[ii].Point.length; jj++)
    { vv=this.Poly[ii].Point[jj];
      if (xxmin>vv.x) xxmin=vv.x;
      if (xxmax<vv.x) xxmax=vv.x;
      if (yymin>vv.y) yymin=vv.y;
      if (yymax<vv.y) yymax=vv.y;
      if (zzmin>vv.z) zzmin=vv.z;
      if (zzmax<vv.z) zzmax=vv.z;
    }
  }
  xxmax-=xxmin;
  yymax-=yymin;
  zzmax-=zzmin;
  vv=xxmax*xxmax+yymax*yymax+zzmax*zzmax;
  if (vv>0.0) ll=Math.sqrt(vv);
  else ll=19.0;
  //BUG: With the next 2 lines active, every time you shift the SVG up, the image gets smaller. If you keep
  //     repeating this sequence (shift up ... draw), the image will shrink down to nothing.
  //if (this.XM<this.YM) this.ZoomAll=1.6*this.XM/ll;
  //else this.ZoomAll=1.6*this.YM/ll;
  this.Dist=2*ll;
  this.ChangeViewer(0, 0);
  return true;
}
function _Scene3DScreenPos(vv)
{ nn=new Vector(0,0,0);
  nn.x= this.sin_Fi*(vv.x-this.Center.x)
       +this.cos_Fi*(vv.y-this.Center.y);
  nn.y=-this.cos_Fi_sin_Th*(vv.x-this.Center.x)
       +this.sin_Fi_sin_Th*(vv.y-this.Center.y)
       -this.cos_Th*(vv.z-this.Center.z);
  nn.z= this.cos_Fi_cos_Th*(vv.x-this.Center.x)
       -this.sin_Fi_cos_Th*(vv.y-this.Center.y)
       -this.sin_Th*(vv.z-this.Center.z);
  if (this.Dist>0.0)
  { nn.x*=this.Dist/(this.Dist-nn.z);
    nn.y*=this.Dist/(this.Dist-nn.z);
  }
//real world to screen:
  nn.Zoom(this.ZoomAll);
  nn.x+=this.XM+this.ShiftX;
  nn.y+=this.YM+this.ShiftY;
  return(nn);
}
function _Scene3DSort()
{ var ii, ll=this.Poly.length, xx, yy, zz;
  if (this.Dist==0.0)
  { for (ii=0; ii<ll; ii++)
    { this.PolyRank[ii][0]=ii;
      this.PolyRank[ii][1]=this.cos_Fi_cos_Th*this.Poly[ii].Center.x*this.OrderWeight.x
        -this.sin_Fi_cos_Th*this.Poly[ii].Center.y*this.OrderWeight.y
        -this.sin_Th*this.Poly[ii].Center.z*this.OrderWeight.z;
    }
  }
  else
  { for (ii=0; ii<ll; ii++)
    { this.PolyRank[ii][0]=ii;
      xx=this.Poly[ii].Center.x*this.OrderWeight.x-this.Viewer.x;
      yy=this.Poly[ii].Center.y*this.OrderWeight.y-this.Viewer.y;
      zz=this.Poly[ii].Center.z*this.OrderWeight.z-this.Viewer.z;
      this.PolyRank[ii][1]=-xx*xx-yy*yy-zz*zz;
    }
  }
  this.PolyRank.sort(_RankSort);
  return true;
}
function _RankSort(ll, rr)
{ if (ll[1]>rr[1]) return(1);
  return(-1);
}
function _Scene3DDraw()
{
  var ii, ll=this.Poly.length;
  var debug=0;
  if(debug) alert("_Scene3DDraw: ll=" + ll + ";  sidx " + sidx );
  this.Light.Normalize();
  for (ii=0; ii<ll; ii++){
    //alert("_Scene3DDraw: ii=" + ii + "; Shape:" + this.Shape[ii] );
    this.Poly[this.PolyRank[ii][0]].Draw(this.Shape[ii]);
  }
  if(ll) {
      if (this.BoundingBox) {
          if(debug) alert("_Scene3DDraw: ii=" + ii + "; calling Draw");
          this.BoundingBox.Draw();
      }
  }
  if(debug) alert("_Scene3DDraw: done ll=" + ll );
  return true;
}
// This function was created to debug why the SVG elements do not show up on Chrome/Safari/Opera using the AppleWebKit
// rendering engine. The Gecko rendering engine works. I thought that maybe while unhiding the hidden row, the SVG
// elements were not getting their visibility set properly.
function CheckForHidden ( element_type )
{
    var elements = document.getElementsByTagName( element_type )
    alert("_Scene3DDelete: 1 ... " + element_type + " elements =" + elements.length );
    for (var i = 0; i < elements.length; i++) {
        var element = elements[i];
        if (element.style.visibility == "hidden" ) {
            alert('element[' + i + "] is id (" + element.id + ") visibility (" + element.style.visibility + ") is hidden" );
        }
    }
}

function _Scene3DDelete()
{ var ii, nn, ss;
  var debug=0;
  if (sidx==1) debug=0;

  if(debug) {
      var objbody = document.getElementById('svg3d_controls');
      alert("_Scene3DDelete: 1 ... sidx=" + sidx );
      if (objbody) {
          var idx;
          alert("_Scene3DDelete: 1 ... svg3d_controls nodes =" + objbody.childNodes.length );
          CheckForHidden ( "input" );
          CheckForHidden ( "svg" );
          CheckForHidden ( "embed" );
          CheckForHidden ( "tr" );
          CheckForHidden ( "td" );
          CheckForHidden ( "th" );
      }
  }
  if (useSVG) ss=this.Parent.GetScene()
  else ss=this.Parent;
  nn=ss.childNodes.length;
  if(debug) alert("_Scene3DDelete: 2 ... " + nn + " nodes" );
  for (ii=0; ii<nn; ii++) {
    ss.removeChild(ss.lastChild);
  }
  if(debug) alert("_Scene3DDelete: 3");
  this.BoundingBox=null;
  this.Poly.length=0;
  if(debug) alert("_Scene3DDelete: 4");
  this.PolyRank.length=0;
  this.Shape.length=0;
  if(debug) alert("_Scene3DDelete: 5");
  this.Callback.length=0;
  if(debug) alert("_Scene3DDelete: 6");
  return true;
}
function _Scene3DZoomUpdate()
{ var ii, ll=this.Poly.length;
  for (ii=0; ii<ll; ii++)
    this.Poly[ii].ZoomUpdate();
  return true;
}
function _Scene3DGetColor(cc0, cc1, nn, pp)
{ var rr, gg, bb, hh="0123456789abcdef";
  var zz, vv;
  if (this.Dist==0.0)
    zz=-this.cos_Fi_cos_Th*nn.x+this.sin_Fi_cos_Th*nn.y+this.sin_Th*nn.z;
  else
    zz=(pp.x-this.Viewer.x)*nn.x+(pp.y-this.Viewer.y)*nn.y+(pp.z-this.Viewer.z)*nn.z;
  if (this.DiffuseLight==1.0)
  { if (zz>0) return(cc0);
    else return(cc1);
  }
  vv=nn.x*this.Light.x+nn.y*this.Light.y+nn.z*this.Light.z;
  if (zz>0)
  { vv*=-1;
    rr=parseInt(cc0.substr(1,2),16);
    gg=parseInt(cc0.substr(3,2),16);
    bb=parseInt(cc0.substr(5,2),16);
  }
  else
  { rr=parseInt(cc1.substr(1,2),16);
    gg=parseInt(cc1.substr(3,2),16);
    bb=parseInt(cc1.substr(5,2),16);
  }
  if (vv<=0)
  { rr=Math.floor(rr*this.DiffuseLight);
    gg=Math.floor(gg*this.DiffuseLight);
    bb=Math.floor(bb*this.DiffuseLight);
  }
  else
  { rr=Math.floor(rr*(vv*(1-this.DiffuseLight)+this.DiffuseLight));
    gg=Math.floor(gg*(vv*(1-this.DiffuseLight)+this.DiffuseLight));
    bb=Math.floor(bb*(vv*(1-this.DiffuseLight)+this.DiffuseLight));
  }
  var ss="#";
  ss+=hh.charAt(Math.floor(rr/16))+hh.charAt(rr%16);
  ss+=hh.charAt(Math.floor(gg/16))+hh.charAt(gg%16);
  ss+=hh.charAt(Math.floor(bb/16))+hh.charAt(bb%16);
  return(ss);
}

function Poly3D(aParentScene, aFrontColor, aBackColor, aStrokeColor, aStrokeWeight)
{ this.Parent=aParentScene;
  this.ClassName="Poly3D";
  this.PhPoint=new Array();
  this.Point=new Array();
  this.Center=new Vector(0.0, 0.0, 0.0);
  this.Normal=new Vector(1.0, 0.0, 0.0);
  this.FrontColor=aFrontColor;
  this.BackColor=aBackColor;
  this.StrokeColor=aStrokeColor;
  this.StrokeWeight=aStrokeWeight;
  this.Visibility="visible";
  this.AddPoint=_Poly3DAddPoint;
  this.SetPoint=_Poly3DSetPoint;
  this.Zoom=_Poly3DZoom;
  this.Shift=_Poly3DShift;
  this.Update=_Poly3DUpdate;
  this.Draw=_Poly3DDraw;
  this.ZoomUpdate=_Poly3DZoomUpdate;
  this.Id="";
  this.Callback=new Array();
  this.Parent.AddPoly(this);
  return true;
}
function _Poly3DAddPoint(xx, yy, zz)
{ vv=this.Parent.Zoom;
  this.PhPoint[this.PhPoint.length]=new Vector(xx, yy, zz);
  this.Point[this.Point.length]=new Vector(xx*vv.x, yy*vv.y, zz*vv.z);
  return true;
}
function _Poly3DSetPoint(ii, xx, yy, zz)
{ vv=this.Parent.Zoom;
  this.PhPoint[ii].x=xx;
  this.PhPoint[ii].y=yy,
  this.PhPoint[ii].z=zz;
  this.Point[ii].x=xx*vv.x;
  this.Point[ii].y=yy*vv.y,
  this.Point[ii].z=zz*vv.z;
  return true;
}
function _Poly3DZoom(ff)
{ for (var ii=0; ii<this.PhPoint.length; ii++)
    this.PhPoint[ii].Zoom(ff);
  for (var ii=0; ii<this.Point.length; ii++)
    this.Point[ii].Zoom(ff);
  this.Update();
  return true;
}
function _Poly3DShift(xx, yy, zz)
{ vv=this.Parent.Zoom;
  for (var ii=0; ii<this.PhPoint.length; ii++)
  { this.PhPoint[ii].x+=xx;
    this.PhPoint[ii].y+=yy;
    this.PhPoint[ii].z+=zz;
  }
  for (var ii=0; ii<this.Point.length; ii++)
  { this.Point[ii].x+=xx*vv.x;
    this.Point[ii].y+=yy*vv.y;
    this.Point[ii].z+=zz*vv.z;
  }
  this.Center.x+=xx*vv.x;
  this.Center.y+=yy*vv.y;
  this.Center.z+=zz*vv.z;
  return true;
}
function _Poly3DUpdate()
{ var ii, ll=this.Point.length;
  this.Center.Zoom(0.0);
  for (ii=0; ii<ll; ii++)
    this.Center.Add(this.Point[ii]);
  this.Center.Zoom(1.0/ll);
  if (ll>2)
  { var xx0=this.Point[0].x-this.Center.x;
    var yy0=this.Point[0].y-this.Center.y;
    var zz0=this.Point[0].z-this.Center.z;
    var xx1=this.Point[1].x-this.Center.x;
    var yy1=this.Point[1].y-this.Center.y;
    var zz1=this.Point[1].z-this.Center.z;
    this.Normal.x=yy0*zz1-zz0*yy1;
    this.Normal.y=zz0*xx1-xx0*zz1;
    this.Normal.z=xx0*yy1-yy0*xx1;
    this.Normal.Normalize();
  }
  return true;
}
function _Poly3DDraw(aShape)
{
  var ii, ss, ll=this.Point.length;
  var vv=this.Parent.ScreenPos(this.Point[0]);
  //alert("Poly3DDraw(" + aShape + ")" );
  if (useSVG)
  { aShape.setAttribute("visibility",this.Visibility);
    ss="M "+parseInt(vv.x)+" "+parseInt(vv.y)+" ";
    for (ii=1; ii<ll; ii++)
    { vv=this.Parent.ScreenPos(this.Point[ii]);
      ss+="L "+parseInt(vv.x)+" "+parseInt(vv.y)+" ";
    }
    ss+="z";
    aShape.setAttribute("d", ss);
    if ((ll>=3)&&(this.FrontColor!=""))
      aShape.setAttribute("fill",this.Parent.GetColor(this.FrontColor, this.BackColor, this.Normal, this.Center));
    else aShape.setAttribute("fill","none");
    if (this.StrokeColor) aShape.setAttribute("stroke",this.StrokeColor);
    else aShape.setAttribute("stroke",this.Parent.GetColor(this.FrontColor, this.BackColor, this.Normal, this.Center));
    aShape.setAttribute("stroke-width",parseInt(this.StrokeWeight));
    aShape.id=this.Id;
    for (var jj in this.Parent.Callback)
    { aShape.removeEventListener(jj, this.Parent.Callback[jj], false);
      if (this.Callback[jj]) aShape.addEventListener(jj, this.Callback[jj], false);
    }
  }
  else
  { if (this.Visibility=="visible")
    { ss="m "+parseInt(vv.x)+","+parseInt(vv.y)+" l";
      for (ii=1; ii<ll-1; ii++)
      { vv=this.Parent.ScreenPos(this.Point[ii]);
        ss+=" "+parseInt(vv.x)+","+parseInt(vv.y)+",";
      }
      vv=this.Parent.ScreenPos(this.Point[ii]);
      ss+=" "+parseInt(vv.x)+","+parseInt(vv.y)+" x e";
      aShape.path=ss;
      if ((ll>=3)&&(this.FrontColor!=""))
      { aShape.fillcolor=this.Parent.GetColor(this.FrontColor, this.BackColor, this.Normal, this.Center);
        aShape.filled=true;
      }
      else aShape.filled=false;
      if (this.StrokeColor) aShape.strokecolor=this.StrokeColor;
      else aShape.strokecolor=this.Parent.GetColor(this.FrontColor, this.BackColor, this.Normal, this.Center);
      aShape.strokeweight=parseInt(this.StrokeWeight);
    }
    else
    { aShape.path="m 0,0 l x e";
      aShape.strokeweight=0;
      aShape.filled=false;
    }
    aShape.id=this.Id;
    for (var jj in this.Parent.Callback)
    { if(this.Callback[jj]) aShape["on"+jj]=this.Callback[jj];
      else aShape["on"+jj]="";
    }
  }
  return true;
}
function _Poly3DZoomUpdate()
{ var ii, ll=this.Point.length, vv=this.Parent.Zoom;
  for (ii=0; ii<ll; ii++)
  { this.Point[ii].x=this.PhPoint[ii].x*vv.x;
    this.Point[ii].y=this.PhPoint[ii].y*vv.y,
    this.Point[ii].z=this.PhPoint[ii].z*vv.z;
  }
  this.Update();
  return true;
}

function _Object3DZoom(ff)
{ for (var ii=0; ii<this.Poly3D.length; ii++) this.Poly3D[ii].Zoom(ff);
  this.Center.Zoom(ff);
  return true;
}
function _Object3DShift(xx, yy, zz)
{ for (var ii=0; ii<this.Poly3D.length; ii++) this.Poly3D[ii].Shift(xx, yy, zz);
  this.Center.x+=xx;
  this.Center.y+=yy;
  this.Center.z+=zz;
  return true;
}
function _Object3DSetFrontColor(aFrontColor)
{ for (var ii=0; ii<this.Poly3D.length; ii++) this.Poly3D[ii].FrontColor=aFrontColor;
  return true;
}
function _Object3DSetBackColor(aBackColor)
{ for (var ii=0; ii<this.Poly3D.length; ii++) this.Poly3D[ii].BackColor=aBackColor;
  return true;
}
function _Object3DSetStrokeColor(aStrokeColor)
{ for (var ii=0; ii<this.Poly3D.length; ii++) this.Poly3D[ii].StrokeColor=aStrokeColor;
  return true;
}
function _Object3DSetStrokeWeight(aStrokeWeight)
{ for (var ii=0; ii<this.Poly3D.length; ii++) this.Poly3D[ii].StrokeWeight=aStrokeWeight;
  return true;
}
function _Object3DSetVisibility(isVisible)
{ for (var ii=0; ii<this.Poly3D.length; ii++)
    this.Poly3D[ii].Visibility=((isVisible)&&(isVisible!="hidden")) ? "visible" : "hidden";
  return true;
}
function _Object3DRotateX(aFi,aCenter)
{ var ii, jj, yy, zz;
  var ccos_Fi=Math.cos(aFi*3.14159265/180);
  var ssin_Fi=Math.sin(aFi*3.14159265/180);
  for (ii=0; ii<this.Poly3D.length; ii++)
  { for (jj=0; jj<this.Poly3D[ii].Point.length; jj++)
    { yy=this.Poly3D[ii].Point[jj].y-aCenter*this.Center.y;
      zz=this.Poly3D[ii].Point[jj].z-aCenter*this.Center.z;
      this.Poly3D[ii].Point[jj].y=aCenter*this.Center.y+ccos_Fi*yy-ssin_Fi*zz;
      this.Poly3D[ii].Point[jj].z=aCenter*this.Center.z+ssin_Fi*yy+ccos_Fi*zz;
    }
    this.Poly3D[ii].Update();
  }
  return true;
}
function _Object3DRotateY(aFi,aCenter)
{ var ii, jj, zz, xx;
  var ccos_Fi=Math.cos(aFi*3.14159265/180);
  var ssin_Fi=Math.sin(aFi*3.14159265/180);
  for (ii=0; ii<this.Poly3D.length; ii++)
  { for (jj=0; jj<this.Poly3D[ii].Point.length; jj++)
    { zz=this.Poly3D[ii].Point[jj].z-aCenter*this.Center.z;
      xx=this.Poly3D[ii].Point[jj].x-aCenter*this.Center.x;
      this.Poly3D[ii].Point[jj].z=aCenter*this.Center.z+ccos_Fi*zz-ssin_Fi*xx;
      this.Poly3D[ii].Point[jj].x=aCenter*this.Center.x+ssin_Fi*zz+ccos_Fi*xx;
    }
    this.Poly3D[ii].Update();
  }
  return true;
}
function _Object3DRotateZ(aFi,aCenter)
{ var ii, jj, xx, yy;
  var ccos_Fi=Math.cos(aFi*3.14159265/180);
  var ssin_Fi=Math.sin(aFi*3.14159265/180);
  for (ii=0; ii<this.Poly3D.length; ii++)
  { for (jj=0; jj<this.Poly3D[ii].Point.length; jj++)
    { xx=this.Poly3D[ii].Point[jj].x-aCenter*this.Center.x;
      yy=this.Poly3D[ii].Point[jj].y-aCenter*this.Center.y;
      this.Poly3D[ii].Point[jj].x=aCenter*this.Center.x+ccos_Fi*xx-ssin_Fi*yy;
      this.Poly3D[ii].Point[jj].y=aCenter*this.Center.y+ssin_Fi*xx+ccos_Fi*yy;
    }
    this.Poly3D[ii].Update();
  }
  return true;
}
function _Object3DSetId(aId)
{ for (var ii=0; ii<this.Poly3D.length; ii++)
  { this.Poly3D[ii].Id=aId;
  }
  return true;
}
function _Object3DSetEventAction(aEvent,aCallback)
{ if(aCallback) this.Parent.Callback[aEvent]=aCallback;
  for (var ii=0; ii<this.Poly3D.length; ii++)
  { this.Poly3D[ii].Callback[aEvent]=aCallback;
  }
  return true;
}

function Box3D(aParentScene, aX0,aY0,aZ0, aX1,aY1,aZ1, aFrontColor, aBackColor, aStrokeColor, aStrokeWeight, aMsg)
{ this.Parent=aParentScene;
  this.ClassName="Box3D";
  this.Center=new Vector((aX0+aX1)/2,(aY0+aY1)/2,(aZ0+aZ1)/2);
  this.FrontColor=aFrontColor;
  this.BackColor=aBackColor;
  this.StrokeColor=aStrokeColor;
  this.StrokeWeight=aStrokeWeight;
  this.Zoom=_Object3DZoom;
  this.Shift=_Object3DShift;
  this.SetFrontColor=_Object3DSetFrontColor;
  this.SetBackColor=_Object3DSetBackColor;
  this.SetStrokeColor=_Object3DSetStrokeColor;
  this.SetStrokeWeight=_Object3DSetStrokeWeight;
  this.SetVisibility=_Object3DSetVisibility;
  this.RotateX=_Object3DRotateX;
  this.RotateY=_Object3DRotateY;
  this.RotateZ=_Object3DRotateZ;
  this.SetPosition=_Box3DSetPosition;
  this.SetId=_Object3DSetId;
  this.SetEventAction=_Object3DSetEventAction;
  this.Poly3D=new Array();
  this.Poly3D[0]=new Poly3D(aParentScene, aFrontColor, aBackColor, aStrokeColor, aStrokeWeight);
  with (this.Poly3D[0])
  { AddPoint(aX0,aY0,aZ0); AddPoint(aX1,aY0,aZ0); AddPoint(aX1,aY1,aZ0); AddPoint(aX0,aY1,aZ0); Update(); }
  this.Poly3D[1]=new Poly3D(aParentScene, aFrontColor, aBackColor, aStrokeColor, aStrokeWeight);
  with (this.Poly3D[1])
  { AddPoint(aX0,aY0,aZ1); AddPoint(aX0,aY1,aZ1); AddPoint(aX1,aY1,aZ1); AddPoint(aX1,aY0,aZ1); Update(); }
  this.Poly3D[2]=new Poly3D(aParentScene, aFrontColor, aBackColor, aStrokeColor, aStrokeWeight);
  with (this.Poly3D[2])
  { AddPoint(aX0,aY0,aZ1); AddPoint(aX1,aY0,aZ1); AddPoint(aX1,aY0,aZ0); AddPoint(aX0,aY0,aZ0); Update(); }
  this.Poly3D[3]=new Poly3D(aParentScene, aFrontColor, aBackColor, aStrokeColor, aStrokeWeight);
  with (this.Poly3D[3])
  { AddPoint(aX0,aY1,aZ0); AddPoint(aX1,aY1,aZ0); AddPoint(aX1,aY1,aZ1); AddPoint(aX0,aY1,aZ1); Update(); }
  this.Poly3D[4]=new Poly3D(aParentScene, aFrontColor, aBackColor, aStrokeColor, aStrokeWeight);
  with (this.Poly3D[4])
  { AddPoint(aX0,aY1,aZ0); AddPoint(aX0,aY1,aZ1); AddPoint(aX0,aY0,aZ1); AddPoint(aX0,aY0,aZ0); Update(); }
  this.Poly3D[5]=new Poly3D(aParentScene, aFrontColor, aBackColor, aStrokeColor, aStrokeWeight);
  with (this.Poly3D[5])
  { AddPoint(aX1,aY0,aZ0); AddPoint(aX1,aY0,aZ1); AddPoint(aX1,aY1,aZ1); AddPoint(aX1,aY1,aZ0); Update(); }
  return true;
}
function _Box3DSetPosition(aX0,aY0,aZ0, aX1,aY1,aZ1)
{ with (this.Poly3D[0])
  { SetPoint(0,aX0,aY0,aZ0); SetPoint(1,aX1,aY0,aZ0); SetPoint(2,aX1,aY1,aZ0); SetPoint(3,aX0,aY1,aZ0); Update(); }
  with (this.Poly3D[1])
  { SetPoint(0,aX0,aY1,aZ1); SetPoint(1,aX1,aY1,aZ1); SetPoint(2,aX1,aY0,aZ1); SetPoint(3,aX0,aY0,aZ1); Update(); }
  with (this.Poly3D[2])
  { SetPoint(0,aX0,aY0,aZ1); SetPoint(1,aX1,aY0,aZ1); SetPoint(2,aX1,aY0,aZ0); SetPoint(3,aX0,aY0,aZ0); Update(); }
  with (this.Poly3D[3])
  { SetPoint(0,aX0,aY1,aZ0); SetPoint(1,aX1,aY1,aZ0); SetPoint(2,aX1,aY1,aZ1); SetPoint(3,aX0,aY1,aZ1); Update(); }
  with (this.Poly3D[4])
  { SetPoint(0,aX0,aY1,aZ0); SetPoint(1,aX0,aY1,aZ1); SetPoint(2,aX0,aY0,aZ1); SetPoint(3,aX0,aY0,aZ0); Update(); }
  with (this.Poly3D[5])
  { SetPoint(0,aX1,aY0,aZ0); SetPoint(1,aX1,aY0,aZ1); SetPoint(2,aX1,aY1,aZ1); SetPoint(3,aX1,aY1,aZ0); Update(); }
  return true;
}

function Line(aParentScene, aX0, aY0, aZ0, aX1, aY1, aZ1, aStrokeColor, aStrokeWeight)
{ this.Parent=aParentScene;
  this.ClassName="Line";
  this.Center=new Vector((aX0+aX1)/2,(aY0+aY1)/2,(aZ0+aZ1)/2);
  this.FrontColor="";
  this.BackColor="";
  this.StrokeColor=aStrokeColor;
  this.StrokeWeight=aStrokeWeight;
  this.Zoom=_Object3DZoom;
  this.Shift=_Object3DShift;
  this.SetStrokeColor=_Object3DSetStrokeColor;
  this.SetStrokeWeight=_Object3DSetStrokeWeight;
  this.SetVisibility=_Object3DSetVisibility;
  this.RotateX=_Object3DRotateX;
  this.RotateY=_Object3DRotateY;
  this.RotateZ=_Object3DRotateZ;
  this.Poly3D=new Array();
  this.Poly3D[0]=new Poly3D(aParentScene, "", "", aStrokeColor, aStrokeWeight);
  this.SetId=_Object3DSetId;
  this.SetEventAction=_Object3DSetEventAction;
  with (this.Poly3D[0])
  { AddPoint(aX0,aY0,aZ0); AddPoint(aX1,aY1,aZ1); Update(); }
  return true;
}

function CoordSys(aParentScene, aStrokeColor)
{ this.Parent=aParentScene;
  this.ClassName="CoordSys";
  this.Center=new Vector(0,0,0);
  this.FrontColor="";
  this.BackColor="";
  this.StrokeColor=aStrokeColor;
  this.StrokeWeight="1";
  this.Zoom=_Object3DZoom;
  this.Shift=_Object3DShift;
  this.SetVisibility=_Object3DSetVisibility;
  this.RotateX=_Object3DRotateX;
  this.RotateY=_Object3DRotateY;
  this.RotateZ=_Object3DRotateZ;
  this.TransformCoord=_CoordSysTransformCoord;
  this.SetId=_Object3DSetId;
  this.SetEventAction=_Object3DSetEventAction;
  this.Poly3D=new Array();
  this.Poly3D[0]=new Poly3D(aParentScene, "", "", this.StrokeColor, this.StrokeWeight);
  with (this.Poly3D[0])
  { AddPoint(0,0,0); AddPoint(1,0,0); AddPoint(0,1,0); AddPoint(0,0,1); Update(); }
  if (aStrokeColor=="") this.SetVisibility(false);
  //This is a helper class to track the coordinates of an object
  //after several rotation, shift and zoom operations
  return true;
}
function _CoordSysTransformCoord(vv)
{ vvv=new Vector(this.Poly3D[0].Point[0].x, this.Poly3D[0].Point[0].y, this.Poly3D[0].Point[0].z);
  vvv.x+=
  (this.Poly3D[0].Point[1].x-this.Poly3D[0].Point[0].x)*vv.x+
  (this.Poly3D[0].Point[2].x-this.Poly3D[0].Point[0].x)*vv.y+
  (this.Poly3D[0].Point[3].x-this.Poly3D[0].Point[0].x)*vv.z;
  vvv.y+=
  (this.Poly3D[0].Point[1].y-this.Poly3D[0].Point[0].y)*vv.x+
  (this.Poly3D[0].Point[2].y-this.Poly3D[0].Point[0].y)*vv.y+
  (this.Poly3D[0].Point[3].y-this.Poly3D[0].Point[0].y)*vv.z;
  vvv.z+=
  (this.Poly3D[0].Point[1].z-this.Poly3D[0].Point[0].z)*vv.x+
  (this.Poly3D[0].Point[2].z-this.Poly3D[0].Point[0].z)*vv.y+
  (this.Poly3D[0].Point[3].z-this.Poly3D[0].Point[0].z)*vv.z;
  return(vvv);
}
