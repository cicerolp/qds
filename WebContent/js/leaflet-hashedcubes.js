function heatmap_layer(value) {

    var canvas_layer = L.tileLayer.canvas({
        minZoom: 0,
        maxZoom: 20,
        unloadInvisibleTiles: false,
        updateWhenIdle: false,
        reuseTiles: false,
        noWrap: true
    });

    canvas_layer.drawTile = function (canvas, coords, zoom) {

        currTile = value;

        var context = canvas.getContext('2d');
        context.globalCompositeOperation = 'lighter';

        var query_map = "/tile/" + value + "/" + coords.x + "/" + coords.y + "/" + zoom + "/" + heatmap_resolution + where + tseries;

        $.ajax({
            type: 'GET',
            url: _queryURL + "/tile" + query_map,
            dataType: "json",
            success: function (data) {
               var entry = {
                  data: data,
                  context: context,
                  tile_x: coords.x,
                  tile_y: coords.y,
                  tile_zoom: zoom,
                  tile_resolution: heatmap_resolution
                };

                color_tile(entry);
            }
        });
    };

    var update = function (options) {
        canvas_layer.redraw();
    };

    callbacks.add(update);

    return canvas_layer;
}

function color_tile(entry) {
    entry.context.clearRect(0, 0, 256, 256);

    var fs = pickDrawFuncs();

    entry.data.forEach(function (d) {

       if (d[2] < entry.tile_zoom + entry.tile_resolution) {
          d[0] = lon2tilex(tilex2lon(d[0] + 0.5, d[2]), entry.tile_zoom + entry.tile_resolution);
          d[1] = lat2tiley(tiley2lat(d[1] + 0.5, d[2]), entry.tile_zoom + entry.tile_resolution);
          d[2] = entry.tile_zoom + entry.tile_resolution;          
       }

       var lon0 = tilex2lon(d[0], d[2]);
       var lat0 = tiley2lat(d[1], d[2]);
       var lon1 = tilex2lon(d[0] + 1, d[2]);
       var lat1 = tiley2lat(d[1] + 1, d[2]);

       var x0 = (lon2tilex(lon0, entry.tile_zoom) - entry.tile_x) * 256;
       var y0 = (lat2tiley(lat0, entry.tile_zoom) - entry.tile_y) * 256;
       var x1 = (lon2tilex(lon1, entry.tile_zoom) - entry.tile_x) * 256;
       var y1 = (lat2tiley(lat1, entry.tile_zoom) - entry.tile_y) * 256;

       var datum = {
          data_zoom: d[2],
          count: d[3],
          tile_zoom: entry.tile_zoom,
          x0: x0,
          y0: y0,
          x1: x1,
          y1: y1
       };

       entry.context.fillStyle = fs.color(d[3]);
       //entry.context.fillStyle = fs.color(d[3] * fs.count_transform(datum));

       fs.draw(entry.context, datum);

        
    });
}

function pickDrawFuncs() {
    var colormaps = {
        ryw: function (count) {
           var lc = Math.log(count + 1) / Math.log(10);

           var r = Math.floor(256 * Math.min(1, lc));
           var g = Math.floor(256 * Math.min(1, Math.max(0, lc - 1)));
           var b = Math.floor(256 * Math.min(1, Math.max(0, lc - 2)));

           return "rgba(" + r + "," + g + "," + b + "," + 1 + ")";
        },
        bbb: d3.scale.linear()
            .domain([1, 100])
            .range(['#000420', '#000D60', '#000D60'])
            .clamp(true)
    };


    var drawfuncs = {
        circle: function draw_circle(context, datum) {
            var radius = 1.0;
            var midx = (datum.x0 + datum.x1) / 2;
            var midy = (datum.y0 + datum.y1) / 2;
            context.beginPath();
            context.arc(midx, midy, radius, 0, 2 * Math.PI);
            context.fill();
        },
        rect: function draw_rect(context, datum) {
            var width = datum.x1 - datum.x0;
            var height = datum.y1 - datum.y0;
            context.fillRect(datum.x0, datum.y0, width, height);
        }
    };

    return {
        draw: drawfuncs[PLOTTING_MODE],
        color: colormaps[PLOTTING_COLOR_SCALE]
    };
}

/* Controls for color scale */
var PLOTTING_MODE = view_schemas[_schema].PLOTTING_MODE;
var PLOTTING_COLOR_SCALE = view_schemas[_schema].PLOTTING_COLOR_SCALE;

var setMode = function (mode) {
    PLOTTING_MODE = mode;
    console.log(PLOTTING_MODE);
    callbacks.fire({ isColorOnly: true });
};

var setScale = function (scale) {
    PLOTTING_COLOR_SCALE = scale;
    console.log(PLOTTING_COLOR_SCALE);
    callbacks.fire({ isColorOnly: true });
};
