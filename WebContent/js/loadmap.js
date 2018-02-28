var DEBUG = false;

var lBound = 0;
var uBound = 23;
var sliderRT = false;

var root = "./api";
var _pipelineURL = root + "/pipeline";
var _queryURL = root + "/query/dataset=" + _schema;

var map;

var ready = false;

function onReady(callback) {
    var intervalID = window.setInterval(checkReady, 500);

    function checkReady() {
        if (ready == false) {
            $.ajax({
                type: 'GET',
                url: root + "/schema/dataset=" + _schema,
                dataType: "json",
                success: function (data) {
                    if (!jQuery.isEmptyObject(data)) {
                        ready = true;

                        for (ptr of data.index_dimensions) {
                            if (ptr.type  == "temporal") {
                                var date = ptr.hint.split("|");

                                lower_bound = new Date(date[0] * 1000);
                                upper_bound = new Date(date[1] * 1000);

                                curr_lower_bound = lower_bound.getTime();
                                curr_upper_bound = upper_bound.getTime();

                                break;
                            }

                        }

                        total_count = data.count;

                        window.clearInterval(intervalID);
                        callback.call(this);
                    }
                },
                error: function (jqXHR, textStatus, errorThrown) {
                    console.log(errorThrown);
                }
            });
        }
    }

}

function show(id, value) {
    document.getElementById(id).style.display = value ? 'block' : 'none';
}

onReady(function () {
    callbacks = $.Callbacks();

    show('container', true);
    show('loading', false);

    loadUi();
    loadMap();

    a_getQuery();
});

var waitForFinalEvent = (function () {
    var timers = {};
    return function (callback, ms, uniqueId) {
        if (!uniqueId) {
            uniqueId = "Don't call this twice without a uniqueId";
        }
        if (timers[uniqueId]) {
            clearTimeout(timers[uniqueId]);
        }
        timers[uniqueId] = setTimeout(callback, ms);
    };
})();

var window_resize = false;

$(window).resize(function () {
    waitForFinalEvent(function () {
        window_resize = true;
        a_getQuery();
    }, 500, "WinResizeEvent");
});

function setProgressBar(count) {
    if (count.length == 0) {
        curr_count = total_count;
    } else if (curr_count != count) {
        curr_count = count;
    } else {
        return;
    }

    $("#progressbar .ui-progressbar-value").animate({
        width: (((curr_count) / total_count) * 100) + "%"
    }, {
        queue: false
    }, {
        duration: 1000
    });

    $("#label").text("Total Count: " + curr_count + " of " + total_count);
}

function updateDataRestrictions() {

    update = false;
    update_tile = false;

    if (curr_heatmap_resolution != heatmap_resolution) {
        update_tile = true;
    }
    heatmap_resolution = curr_heatmap_resolution;

    curr_region = "";
    if (marker != null) {


        for (var i = 0; i < marker.length; i++) {
            if (marker[i] == null) continue;

            var b = L.latLngBounds(tiles[i].p0, tiles[i].p1);

            var lat0 = b._northEast.lat;
            var lon0 = b._southWest.lng;
            var lat1 = b._southWest.lat;
            var lon1 = b._northEast.lng;

            var z = map.getZoom() + 8;

            var x0 = roundtile(lon2tilex(lon0, z), z);
            var x1 = roundtile(lon2tilex(lon1, z), z);

            if (x0 > x1) {
                x0 = 0;
                x1 = Math.pow(2, z);
            }

            // [dimension_name].region.([x0]:[y0]:[x1]:[y1]:[z])
            curr_region += "/const=" + currTileValue + ".region.("
                + x0 + ":"
                + roundtile(lat2tiley(lat0, z), z) + ":"
                + x1 + ":"
                + roundtile(lat2tiley(lat1, z), z) + ":"
                + z + ")"
        }
    }

    if (curr_region != region) {
        update = true;
    }
    region = curr_region;

    curr_where = "";

    _view.views.forEach(function (entry) {
        if (entry.on_menu) {
            var restriction = "/const=" + entry.field.name + ".values.(";

            var values = "";

            $("#tabs-" + entry.field.name + "-checkboxes" + " :checked").each(function () {
                values += parseInt($(this).val()) + ":";
            });
            values = values.substring(0, values.length - 1);

            if (values != "") {
                restriction += values + ")"
            } else {
                restriction += "all" + ")"
            }

            curr_where += restriction;
        }
    });

    if (curr_where != where) {
        update = true;
        update_tile = true;
    }
    where = curr_where;

    curr_tseries = "";

    _view.views.forEach(function (entry) {
        if (entry.type == "time-series") {

            var tseries_from = Math.floor(Math.max(lower_bound.getTime(), curr_lower_bound) / 1000);
            var tseries_to = Math.ceil(Math.min(upper_bound.getTime(), curr_upper_bound) / 1000);

            curr_tseries += "/const=" + entry.field.name + ".interval.(" + tseries_from + ":" + tseries_to + ")";
        }
    });

    if (curr_tseries != tseries) {
        update = true;
        update_tile = true;
    }
    tseries = curr_tseries;

    if (window_resize) {
        update = true;
    }
    window_resize = false;
}

function a_getQuery() {
    if ($("#play").val() == 0) {
        return;
    }

    updateDataRestrictions();

    if (update_tile) callbacks.fire(heatmap_resolution + where + tseries);

    if (update) {
        var query = "/aggr=count" + region + tseries + where;
        $.ajax({
            type: 'GET',
            url: _queryURL + query,
            dataType: "json",
            success: function (data) {
                setProgressBar(data);
            },
            error: function (jqXHR, textStatus, errorThrown) {
                console.log(errorThrown);
            }
        });

        _view.views.forEach(function (entry) {
            switch (entry.type) {
                /*case "mysql": {
                    var query = "/count/mysql" + tseries + region + where;
                    $.ajax({
                        type: 'GET',
                        url: _queryURL + query,
                        dataType: "json",
                        success: function (data) {
                            loadMySQLPanel(data, entry);
                        },
                        error: function (jqXHR, textStatus, errorThrown) {
                            console.log(errorThrown);
                        }
                    });
                }
                    break;*/

                case "histogram": {
                    var query = "/aggr=count" + region + where + tseries + "/group=" + entry.field.name;
                    $.ajax({
                        type: 'GET',
                        url: _queryURL + query,
                        dataType: "json",
                        success: function (data) {
                            setHistogramData(data, entry);
                        },
                        error: function (jqXHR, textStatus, errorThrown) {
                            console.log(errorThrown);
                        }
                    });
                }
                    break;

                case "time-series": {
                    var query = "/aggr=count" + region + where + tseries + "/group=" + entry.field.name;
                    $.ajax({
                        type: 'GET',
                        url: _queryURL + query,
                        dataType: "json",
                        success: function (data) {
                            loadLineChart(data, entry);
                        },
                        error: function (jqXHR, textStatus, errorThrown) {
                            console.log(errorThrown);
                        }
                    });
                }
                    break;

                /*case "binned-scatterplot": {
                    var query = "/count/scatter/field/" + get_id(entry.field_x.name) + "/field/" + get_id(entry.field_y.name) + region;

                    $.ajax({
                        type: 'GET',
                        url: _queryURL + query,
                        dataType: "json",
                        success: function (data) {
                            setSctterChart(data, entry);
                        },
                        error: function (jqXHR, textStatus, errorThrown) {
                            console.log(errorThrown);
                        }
                    });
                }
                    break;*/
            }
        });
    }
}
