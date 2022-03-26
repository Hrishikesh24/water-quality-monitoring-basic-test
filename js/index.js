var tds = [];
var ph = [];
var temp = [];
var timestamp = [];
var firebase = new Firebase("wqmtest-f7ebb-default-rtdb.firebaseio.com");



firebase.on('value', function (snapshot) {
  for (let i in snapshot.val().tds) {
    tds.push(snapshot.val().tds[i]);
  }
  for (let i in snapshot.val().ph) {
    ph.push(snapshot.val().ph[i]);
  }
  for (let i in snapshot.val().temp) {
    temp.push(snapshot.val().temp[i]);
  }
  for (let i in snapshot.val().timestamp) {
    tS = new Date(snapshot.val().timestamp[i] * 1000).toISOString().slice(0, 19).replace('T', ' ')
    timestamp.push(tS);
  }

  tds = tds.slice(tds.length - 5, tds.length);
  ph = ph.slice(ph.length - 5, ph.length);
  temp = temp.slice(temp.length - 5, temp.length);

  timestamp = timestamp.slice(timestamp.length - 5, timestamp.length);

  var tdsItem = `<div class="h-full shadow-2xl bg-blue-400 bg-opacity-100 px-10 pt-16 pb-16 rounded-3xl overflow-hidden text-left relative">
  <h2 class="tracking-widest text-lg title-font font-bold text-gray-100 mb-1">Total Dissolved Solids (TDS)</h2>
  <h1 class="title-font sm:text-9xl text-6xl font-black text-white">${tds[tds.length - 1]}</h1>
  <p class="leading-relaxed font-medium mb-3">PPM</p>
  <p class="leading-relaxed font-thin mb-3">Last Updated: ${timestamp[timestamp.length - 1]}</p>             
</div>`;
  document.getElementById("tdsLatest").innerHTML = tdsItem;

  var phItem = `<div class="h-full shadow-2xl bg-indigo-400 bg-opacity-100 px-10 pt-16 pb-16 rounded-3xl overflow-hidden text-left relative">
  <h2 class="tracking-widest text-lg title-font font-bold text-gray-100 mb-1">pH</h2>
  <h1 class="title-font sm:text-9xl text-6xl font-black text-white">${ph[ph.length - 1]}</h1>
  <p class="leading-relaxed font-thin mb-3">Last Updated: ${timestamp[timestamp.length - 1]}</p>              
</div>`;

  document.getElementById("phLatest").innerHTML = phItem;

  var tempItem = `<div class="h-full shadow-2xl bg-indigo-600 bg-opacity-100 px-10 pt-16 pb-16 rounded-3xl overflow-hidden text-left relative">
<h2 class="tracking-widest text-lg title-font font-bold text-gray-100 mb-1">Relative Temperature</h2>
<h1 class="title-font sm:text-9xl text-6xl font-black text-white">${temp[temp.length - 1]}</h1>
<p class="leading-relaxed font-medium mb-3">°C</p>
<p class="leading-relaxed font-thin mb-3">Last Updated: ${timestamp[timestamp.length - 1]}</p>                
</div>`;

  document.getElementById("tempLatest").innerHTML = tempItem;
  // gas.forEach((o, i, a) => a[i] = o*100/1024);
  if (Methane.length < 5 && timestamp.length < 5) {
    document.getElementById("att").innerHTML = `<p class="font-thin text-gray-600">The chart will be displayed once the SM saves 5 datapoints</p>`;
  }
  else {
    document.getElementById("att").innerHTML = `<p class="font-thin text-gray-600"></p>`;
    drawGraph(Methane, Temp, Hum, sTemp, timestamp);
  }
});

function drawGraph(tds, ph, temp, timestamp) {
  //var labels = [0, 1, 2, 3, 4 , 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19];
  var labels = timestamp;
  var ctx = document.getElementById("myChart").getContext('2d');

  var myChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: labels,
      datasets: [{
        label: "TDS [PPM]",
        labelString: "PPM",

        borderColor: 'rgb(0, 155, 255)',
        backgroundColor: 'rgb(0, 155, 255)',
        fill: false,
        data: tds,
        yAxisID: "y-axis-temp",

      },
      {
        label: "pH",
        labelString: "pH",

        borderColor: 'rgb(0, 99, 132)',
        backgroundColor: 'rgb(0, 99, 132)',
        fill: false,
        data: ph,
        yAxisID: "y-axis-temp",

      },
      {
        label: "Temperature [°C]",
        labelString : "°C",
        borderColor: 'rgb(255, 99, 132)',
        backgroundColor: 'rgb(255, 99, 132)',
        fill: false,
        data: temp,
          yAxisID: "y-axis-temp",
        }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      hoverMode: 'index',
      stacked: false,
      title: {
        display: false,
        text: 'level'
      },
      scales: {
        yAxes: [{
          type: "linear",
          display: true,
          position: "left",
          id: "y-axis-temp",
          ticks: {
            beginAtZero: true,
            suggestedMax: 50
          }
        }],
      }
    }
  });
}
