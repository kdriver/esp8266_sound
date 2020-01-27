
const char graph[] PROGMEM = R"rawliteral(
 <!DOCTYPE HTML><html><head>
    <title>Boiler readings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta http-equiv="refresh" content="20">
    </head>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.2/Chart.js"></script>
    <body>
    <canvas id="myChart" width="400" height="400"></canvas>
<script>
    var ctx = document.getElementById('myChart').getContext('2d');
    ctx.canvas.parentNode.style.height = "500px";
    var myChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: %s,
            datasets: [{
                label: 'sound level',
                data: %s
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                yAxes: [{
                    ticks: {
                        beginAtZero: true
                    }
                }]
            }
        }
    });
    </script>
    </body>
    </html>)rawliteral";


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head>
  <body>
  abs_average %d boiler_on_threshold %d  boiler_status %s
  </p>
  <form action="/get">
    Enter a value : <input type="text" name="boiler_on_threshold">
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral";

const char* PARAM_INPUT_1 = "boiler_on_threshold";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}