var leftRac = document.getElementById('ractangel')
var leftTri = document.getElementById('triangle')
var racCol = leftRac.style.backgroundColor;

var leftRac1 = document.getElementById('ractangel1')
var leftTri1 = document.getElementById('triangle1')
var racCol1 = leftRac.style.backgroundColor;

function changeColor(){
  leftRac.style.backgroundColor = '#1D1E21';
  leftTri.style.borderTop = "80px solid #1D1E21";
  leftTri.style.borderBottom = "80px solid transparent";
  leftTri.style.borderLeft = "80px solid #1D1E21";
  leftTri.style.borderRight = "80px solid transparent";
}

function resetColor(){
  leftRac.style.backgroundColor = racCol;
  leftTri.style.borderTop = "80px solid #c0c0c0";
  leftTri.style.borderBottom = "80px solid transparent";
  leftTri.style.borderLeft = "80px solid #c0c0c0";
  leftTri.style.borderRight = "80px solid transparent";
}

function changeColor1(){
  leftRac1.style.backgroundColor = '#1D1E21';
  leftTri1.style.borderTop = "80px solid transparent";
  leftTri1.style.borderBottom = "80px solid #1D1E21";
  leftTri1.style.borderLeft = "80px solid transparent";
  leftTri1.style.borderRight = "80px solid #1D1E21";
}

function resetColor1(){
  leftRac1.style.backgroundColor = racCol;
  leftTri1.style.borderTop = "80px solid transparent";
  leftTri1.style.borderBottom = "80px solid #c0c0c0";
  leftTri1.style.borderLeft = "80px solid transparent";
  leftTri1.style.borderRight = "80px solid #c0c0c0";
}
