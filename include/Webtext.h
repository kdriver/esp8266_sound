const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
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