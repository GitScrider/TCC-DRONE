/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-mpu-6050-web-server/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/



let scene, camera, rendered, cube;

var ctrlgraphdisplay = 0;
var graphtime = 0;

var thetadata = [];
var phidata = [];
var psidata = [];
var zdata = [];
var signal1 = [];
var signal2 = [];
var signal3 = [];
var signal4 = [];

const thetalabel ='θ';
const philabel ='Φ';
const psilabel ='Ψ';
const Zlabel ='Z';


function parentWidth(elem) {
  return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
  return elem.parentElement.clientHeight;
}

function init3D(){
  scene = new THREE.Scene();
  scene.background = new THREE.Color(0xffffff);

  camera = new THREE.PerspectiveCamera(75, parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube")), 0.1, 1000);

  renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));

  document.getElementById('3Dcube').appendChild(renderer.domElement);

  // Create a geometry
  const geometry = new THREE.BoxGeometry(5, 1, 4);

  // Materials of each face
  var cubeMaterials = [
    new THREE.MeshBasicMaterial({color:0x008000}),
    new THREE.MeshBasicMaterial({color:0x00bd00}),
    new THREE.MeshBasicMaterial({color:0x00e000}),
    new THREE.MeshBasicMaterial({color:0x008000}),
    new THREE.MeshBasicMaterial({color:0x00bd00}),
    new THREE.MeshBasicMaterial({color:0x00e000}),
  ];

  const material = new THREE.MeshFaceMaterial(cubeMaterials);

  cube = new THREE.Mesh(geometry, material);
  scene.add(cube);
  camera.position.z = 5;
  renderer.render(scene, camera);
}

// Resize the 3D object when the browser window changes size
function onWindowResize(){
  camera.aspect = parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube"));
  //camera.aspect = window.innerWidth /  window.innerHeight;
  camera.updateProjectionMatrix();
  //renderer.setSize(window.innerWidth, window.innerHeight);
  renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));

}

window.addEventListener('resize', onWindowResize, false);

// Create the 3D representation
init3D();

// Create events for the sensor readings
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('gyro_readings', function(e) {
    //console.log("gyro_readings", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("gyroX").innerHTML = obj.gyroX;
    document.getElementById("gyroY").innerHTML = obj.gyroY;
    document.getElementById("gyroZ").innerHTML = obj.gyroZ;

    // Change cube rotation after receiving the readinds
    cube.rotation.x = obj.gyroY;
    cube.rotation.z = obj.gyroX;
    cube.rotation.y = obj.gyroZ;
    renderer.render(scene, camera);
  }, false);

  source.addEventListener('temperature_reading', function(e) {
    //console.log("temperature_reading", e.data);
    document.getElementById("temp").innerHTML = e.data;
  }, false);

  source.addEventListener('accelerometer_readings', function(e) {
    //console.log("accelerometer_readings", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("accX").innerHTML = obj.accX;
    document.getElementById("accY").innerHTML = obj.accY;
    document.getElementById("accZ").innerHTML = obj.accZ;
  }, false);

  source.addEventListener('distance_reading',function(e){
    //console.log("distance_reading", e.data);
    document.getElementById("dist").innerHTML = e.data;
  },false);

  source.addEventListener('graph_reading',function(e){
    console.log("graph_reading", e.data);
    var obj = JSON.parse(e.data);

    if(ctrlgraphdisplay===30){
      
      labels.shift();
      signal1.shift();
      signal2.shift();
      signal3.shift();
      signal4.shift();
      thetadata.shift();
      phidata.shift();
      psidata.shift();
      zdata.shift();

    }else{
      ctrlgraphdisplay=ctrlgraphdisplay+1;
    }

    labels.push(graphtime.toFixed(1));
    
    phidata.push(obj.gyroX);
    thetadata.push(obj.gyroY);
    psidata.push(obj.gyroZ);
    zdata.push(obj.Z);
    signal1.push(obj.Signal1);
    signal2.push(obj.Signal2);
    signal3.push(obj.Signal3);
    signal4.push(obj.Signal4);
  
    graphtime = graphtime+0.1;

    data.datasets[0].data=signal1;
    data.datasets[1].data=signal2;
    data.datasets[2].data=signal3;
    data.datasets[3].data=signal4;
    data.datasets[4].data = zdata;
    data.datasets[5].data = thetadata;
    data.datasets[6].data = phidata;
    data.datasets[7].data = psidata;

    ThetaChart.destroy();
    ThetaChart = new Chart(
      document.getElementById('ThetaChart'),
      config
    );

   

  },false);

}

function resetPosition(element){
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/"+element.id, true);
  console.log(element.id);
  xhr.send();
}

function decreaseengine(element){
  var xhr = new XMLHttpRequest();
  xhr.open("GET","/"+element.id,true);
  console.log(element.id);
  xhr.send();
}

function increaseengine(element){
  var xhr = new XMLHttpRequest();
  xhr.open("GET","/"+element.id,true);
  console.log(element.id);
  xhr.send();
}

function ESCCalibration(element){
  var xhr = new XMLHttpRequest();
  xhr.open("GET","/"+element.id,true);
  console.log(element.id);
  xhr.send();
}

var labels = [];

var data = {
  labels: labels,
  datasets: [{
    label: 'Sinal 1',
    backgroundColor: 'transparent',
    borderColor: 'red',
    data: [],
  },
  {
    label: 'Sinal 2',
    backgroundColor: 'transparent',
    borderColor: 'yellow',
    data: [],
  },
  {
    label: 'Sinal 3',
    backgroundColor: 'transparent',
    borderColor: 'green',
    data: [],
  },
  {
    label: 'Sinal 4',
    backgroundColor: 'transparent',
    borderColor: 'blue',
    data: [],
  },
  {
    label: Zlabel,
    backgroundColor: 'transparent',
    borderColor: 'black',
    data: [],
  },
  {
    label: thetalabel,
    backgroundColor: 'transparent',
    borderColor: 'orange',
    data: [],
  },
  {
    label: philabel,
    backgroundColor: 'transparent',
    borderColor: 'cyan',
    data: [],
  },
  {
    label: psilabel,
    backgroundColor: 'transparent',
    borderColor: 'purple',
    data: [],
  }
  
]
};

var config = {
  type: 'line',
  data: data,
  options: {
      animation: false,
      elements:{
        point:{
          radius:0
        },
        line:{
          tension:0
        }
      },
      scales: {
        y: {
            type: 'linear',
            min: -100,
            max: 100,
           },
    }

  }
};

var ThetaChart = new Chart(
  document.getElementById('ThetaChart'),
  config
);

