function heatmap_layer(value) {

    var canvas_layer = L.tileLayer.canvas({
        minZoom: 0,
        maxZoom: 18,
        unloadInvisibleTiles: false,
        updateWhenIdle: false,
        reuseTiles: false,
        noWrap: true
    });

    canvas_layer.drawTile = function (canvas, coords, zoom) {

        currTileValue = value;

        var context = canvas.getContext('2d');
        context.globalCompositeOperation = 'color';

        // [dimension_name].tile.([x]:[y]:[z]:[resolution])

        var query_const = where + tseries;

        var map_group = "/group=" + value;
        var map_const = "/const=" + value + ".tile.(" + coords.x + ":" + coords.y + ":" + zoom + ":" + heatmap_resolution + ")";

        $.ajax({
            type: 'GET',
            
            url:

	    _pipelineURL +
	    "/join=right_join" + 
                "/threshold=" + curr_join_threshold +
                "/source/aggr=average.dep_delay_g/dataset=" +                 
                _schema + map_const + map_group +
		"/const=crs_dep_time.interval.(1511481600:1511568000)" +
                	    
                "/destination/aggr=inverse.dep_delay_t.($)/dataset=" + 
                _schema + map_const + map_group + query_const,

	    

	    // _pipelineURL + 
            //     "/join=right_join" + 
            //     "/threshold=" + curr_join_threshold +                
            //     "/source/aggr=average.total_amount_g/dataset=" +                 
            //     _schema + map_const + map_group + 
	    // 	"/const=pickup_datetime.interval.(999388800:999475200)" +

	    
            //     "/destination/aggr=inverse.total_amount_t.($)/dataset=" + 
            //     _schema + map_const + map_group + query_const,

            dataType: "json",
            success: function (data) {
                var entry = {
                    data: data[0],
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
    if (typeof entry.data == 'undefined')
        return;

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

	if(d[4] < 0)
	    console.log("****",d);
	
        var datum = {
            data_zoom: d[2],
            count: d[4],
            tile_zoom: entry.tile_zoom,
            x0: x0,
            y0: y0,
            x1: x1,
            y1: y1
        };

        entry.context.fillStyle = fs.color(d[4]);
        fs.draw(entry.context, datum);
    });
}

function pickDrawFuncs() {
    var colormaps = {
        ryw: function (count) {
            var lc = Math.log(count) / Math.log(100);

            var r = Math.floor(256 * Math.min(1, lc));
            var g = Math.floor(256 * Math.min(1, Math.max(0, lc - 1)));
            var b = Math.floor(256 * Math.min(1, Math.max(0, lc - 2)));

            return "rgba(" + r + "," + g + "," + b + "," + 1.0 + ")";
        },
        bbb: d3.scale.threshold()
            .domain([0.0, 0.25, 0.75])
            .range(['rgba(152,78,163,0.75)', 'rgba(228,26,28,0.75)','rgba(55,126,184,0.75)','rgba(77,175,74,0.75)']),

        debug: function (count) {
            return "rgba(256,256,256,1.0)";
        },
    };


    var drawfuncs = {
        circle: function draw_circle(context, datum) {
            var radius = 2.0;
            var midx = (datum.x0 + datum.x1) / 2;
            var midy = (datum.y0 + datum.y1) / 2;
            context.beginPath();
            context.arc(midx, midy, radius, 0, 2 * Math.PI);
            context.fill();
        },
        rect: function draw_rect(context, datum) {
            const size_px = 0.0;
            var width = datum.x1 - datum.x0;
            var height = datum.y1 - datum.y0;
            context.fillRect(datum.x0 - size_px, datum.y0 - size_px, width + size_px, height + size_px);
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
    callbacks.fire({isColorOnly: true});
};

var setScale = function (scale) {
    PLOTTING_COLOR_SCALE = scale;
    console.log(PLOTTING_COLOR_SCALE);
    callbacks.fire({isColorOnly: true});
};
