L.GridLayer.CanvasCircles = L.GridLayer.extend({
    setOpacity(v){
	this.options.opacity = v;
	this.redrawTiles();
    },
    setResolution(v){
	this.options.resolution = v;
	this.redraw();
    },
    setQuantileMapQuantile(v){
	this.options.quantileQuery = v;
	this.redraw();
    },
    setInverseQuantileMapQuantile(v){
	this.options.inverseQuantileQuery = v;
	this.redraw();
    },
    setQueryThreshold(v){
	this.options.queryThreshold = v;
	this.redraw();
    },
    setInverseQuantileFilter(min,max){
	this.options.inverseQuantileFilter = [min,max];
	this.redraw();
    },
    setState(v){
	this.options.state = v;
	this.redraw();
    },
    setMyColorScaleDomain(domain){
	this.options.myColorScaleDomain = domain;
    },
    myColorScale:function(value){
	// var opacity = this.options.opacity
	// var lc = Math.log(value + 1) / Math.log(10);
        // var r = Math.floor(255 * Math.min(1, lc));
	// var g = Math.floor(255 * Math.min(1, Math.max(0, lc - 1)));
	// var b = Math.floor(255 * Math.min(1, Math.max(0, lc - 2)));
	// var a = opacity;
        //return  "rgba(" + ([r, g, b, a].join(",")) + ")";
	var scale = d3.scaleQuantile().domain(this.options.myColorScaleDomain).range(['#ffffcc','#ffeda0','#fed976','#feb24c','#fd8d3c','#fc4e2a','#e31a1c','#b10026']);
	return scale(value);
;
    },
    normalizedColorScale:function(value){
	var opacity = this.options.opacity
	var scale = inverseQuantileScale;//["rgba(255,0,0,"+opacity+")","rgba(0,0,255,"+opacity+")"]);
	console.log(value,scale(value));
	return scale(value);
    },
    colorTile: function(tile, coords,totalResolution){
	//
	var layer = this;
        var ctx = tile.getContext('2d');

	//
        var tileSize = this.getTileSize();
	var resolution = this.options.resolution;
	var numPixels = 2**resolution;
	var pixelWidth  = tileSize.x / numPixels;
	var pixelHeight = tileSize.y / numPixels;
	
	//
	tile.data.forEach(function(crudePixel){
	    var pixel = crudePixel;
	    if(pixel[2] != totalResolution){
		var pixelLatLng = [tilex2lon(pixel[0],pixel[2]),tiley2lat(pixel[1],pixel[2])];
		pixel = [lon2tilex(pixelLatLng[0],totalResolution),lat2tiley(pixelLatLng[1],totalResolution)];
	    }

	    var pixelInLocalCoords = [pixel[0]-coords[0],pixel[1]-coords[1]];
	    var rgba;
	    if(layer.options.state == "inverse_quantile"){
		if(layer.options.inverseQuantileFilter[0] > crudePixel[3] ||
		   layer.options.inverseQuantileFilter[1] < crudePixel[3])
		    return;
		//debugger
		rgba = layer.normalizedColorScale(crudePixel[3]);
	    }
	    else
		rgba = layer.myColorScale(pixel[3]);
	    ctx.fillStyle = rgba;
	    ctx.fillRect(pixelInLocalCoords[0]*pixelWidth, pixelInLocalCoords[1]*pixelHeight , pixelWidth, pixelHeight);
	});
    },
    createTile: function (coords,done) {
	var layer = this;
        var tile = document.createElement('canvas');
	tile.data = [];
	//
        var tileSize = layer.getTileSize();
        tile.setAttribute('width', tileSize.x);
        tile.setAttribute('height', tileSize.y);
	//
	var resolution = this.options.resolution;
	var totalResolution = resolution + coords.z;	
	var tileLatLng = [tilex2lon(coords.x,coords.z),tiley2lat(coords.y,coords.z)];
	var tileInTotalResolution = [lon2tilex(tileLatLng[0],totalResolution),lat2tiley(tileLatLng[1],totalResolution)];
	
	//
	var fCallback = function(queryReturn,myQ){
	    
	    var result = [];
	    //TODO: remove this fix when cicero fix the result standard
	    if(queryReturn.length > 0)
		result = queryReturn[0];
	    
	    if(layer.options.state == "count"){
		//
		if(myQ.threshold > 0){
		    result = result.filter(d=>d[3] >= myQ.threshold)
		}
		//
		tile.data = result;
	    }
	    else if(layer.options.state == "quantile"){
		//
		var counts = queryReturn[1];
		if(myQ.threshold > 0){
		    result = result.filter((d,i)=>counts[i][3] >= myQ.threshold)
		}
		result = result.map(entry=>[entry[0],entry[1],entry[2],entry[4]]);
		tile.data = result;
		//TODO: set proper scale
	    }
	    else if(layer.options.state == "inverse_quantile"){
		//
		var counts = queryReturn[1];
		if(myQ.threshold > 0){
		    result = result.filter((d,i)=>counts[i][3] >= myQ.threshold)
		}
		//
		result = result.map(entry=>[entry[0],entry[1],entry[2],entry[4]]);
		tile.data = result;
		//TODO: set proper scale
	    }
	    else if(layer.options.state == "average"){
		//
		var counts = queryReturn[1];
		if(myQ.threshold > 0){
		    result = result.filter((d,i)=>counts[i][3] >= myQ.threshold)
		}
		//
		tile.data = result;
		//TODO: set proper scale
	    }
	    else if(layer.options.state == "quantile_range"){
		//
		var auxMap = {};
		var consolidatedData = [];
		result.forEach(entry=>{
		    var key = entry[0] + "_" + entry[1] + "_" + entry[2];
		    if(key in auxMap){
			consolidatedData.push([entry[0],entry[1],entry[2],entry[4]-auxMap[key][4]]);
		    }
		    else{
			auxMap[key] = entry;
		    }
		});
		//
		if(myQ.threshold > 0){
		    var counts = queryReturn[1];
		    consolidatedData = consolidatedData.filter((d,i)=>counts[i][3] >= myQ.threshold)
		}
		//
		tile.data = consolidatedData;
		//TODO: set proper scale		
	    }
	    else{
		console.log("not here", layer.options.state);
		if(queryReturn[0].length > 0)
		debugger
	    }

	    layer.colorTile(tile,tileInTotalResolution,totalResolution);	    	    
            done(null, tile);	// Syntax is 'done(error, tile)'
	};
	//
	var query = undefined;
	if(layer.options.state == "count"){
	    query = new NDSQuery(datasetInfo.datasetName,activeSpatialDimension,fCallback);
	    query.addAggregation("count");
	    query.addConstraint("time_interval",activeTemporalDimension,timeConstraint);
	    query.addConstraint("tile",activeSpatialDimension,{"x":coords.x,"y":coords.y,"z":coords.z,"resolution":resolution});
	}
	else if(layer.options.state == "average"){
	    query = new NDSQuery(datasetInfo.datasetName,activeSpatialDimension,fCallback);
	    query.addAggregation("average",activePayloadDimension+"_g");
	    query.addAggregation("count");
	    query.addConstraint("time_interval",activeTemporalDimension,timeConstraint);	   
	    query.addConstraint("tile",activeSpatialDimension,{"x":coords.x,"y":coords.y,"z":coords.z,"resolution":resolution});
	}
	else if(layer.options.state == "quantile"){
	    query = new NDSQuery(datasetInfo.datasetName,activeSpatialDimension,fCallback);
	    query.addAggregation("quantile",activePayloadDimension + "_t");
	    query.addAggregation("count",activePayloadDimension + "_t");
	    query.addConstraint("time_interval",activeTemporalDimension,timeConstraint);	   
	    query.addConstraint("tile",activeSpatialDimension,{"x":coords.x,"y":coords.y,"z":coords.z,"resolution":resolution});
	    query.setPayload({"quantiles":[layer.options.quantileQuery]});
	}
	else if(layer.options.state == "quantile_range"){
	    query = new NDSQuery(datasetInfo.datasetName,activeSpatialDimension,fCallback);
	    query.addAggregation("quantile",activePayloadDimension + "_t");
	    query.addAggregation("count");
	    query.addConstraint("time_interval",activeTemporalDimension,timeConstraint);	   
	    query.addConstraint("tile",activeSpatialDimension,{"x":coords.x,"y":coords.y,"z":coords.z,"resolution":resolution});
	    query.setPayload({"quantiles":[0.25,0.75]});
	}
	else if(layer.options.state == "inverse_quantile"){
	    query = new NDSQuery(datasetInfo.datasetName,activeSpatialDimension,fCallback);
	    query.addAggregation("inverse_quantile",activePayloadDimension + "_t");
	    query.addAggregation("count");
	    query.addConstraint("time_interval",activeTemporalDimension,timeConstraint);	   
	    query.addConstraint("tile",activeSpatialDimension,{"x":coords.x,"y":coords.y,"z":coords.z,"resolution":resolution});
	    query.setPayload({"inverse_quantile":layer.options.inverseQuantileQuery});
	}

	//
	//query.addConstraint("categorical","unique_carrier",{"values":["1432"]});
	query.addConstraint("categorical","unique_carrier",{"values":Object.keys(mapIndexToName)});
	
	query.setThreshold(this.options.queryThreshold);
	query.tile = [tileInTotalResolution[0],tileInTotalResolution[1],totalResolution];
	//console.log(query.toString());
	ndsInterface.query(query);
	
        return tile;
    	
    },
    redrawTiles:function(){
	var layer = this;
	for(var tileKey in this._tiles){
	    var tile = this._tiles[tileKey];
	    var canvas = tile.el;
	    var coords = tile.coords;
	    //
	    var resolution = this.options.resolution;
	    var totalResolution = resolution + coords.z;	
	    var tileLatLng = [tilex2lon(coords.x,coords.z),tiley2lat(coords.y,coords.z)];
	    var tileInTotalResolution = [lon2tilex(tileLatLng[0],totalResolution),lat2tiley(tileLatLng[1],totalResolution)];
	    //
            var tileSize = this.getTileSize();
            var ctx = canvas.getContext('2d');
	    ctx.clearRect(0, 0, tileSize.x,tileSize.y);

	    //
	    layer.colorTile(canvas,tileInTotalResolution,totalResolution);
	}
    }
});

L.gridLayer.canvasCircles = function(opts) {
    return new L.GridLayer.CanvasCircles(opts);
};


L.GridLayer.CanvasLayer = L.GridLayer.extend({
    createTile: function (coords, done) {
	if(true){
	    var tile = document.createElement('canvas');

            var tileS1ize = this.getTileSize();
            tile.setAttribute('width', tileSize.x);
            tile.setAttribute('height', tileSize.y);

            var ctx = tile.getContext('2d');

            // Draw whatever is needed in the canvas context
            // For example, circles which get bigger as we zoom in
            ctx.beginPath();
            ctx.arc(tileSize.x/2, tileSize.x/2, 4 + coords.z*4, 0, 2*Math.PI, false);
            ctx.fill();
            return tile;
	}
    }
});

//http://10.0.100.237:8585/q(visits.b('location',dive(tile2d(4,3,7),2),'img8'))
//http://10.0.100.237:8585/q(visits.b('location',dive(tile2d(10,412,488),0),'img8'))

L.gridLayer.canvasLayer = function(opts) {
    return new L.GridLayer.CanvasLayer(opts);
};

L.GridLayer.DebugCoords = L.GridLayer.extend({
    createTile: function (coords) {
        var tile = document.createElement('div');
        tile.innerHTML = [coords.x, coords.y, coords.z].join(', ');
        tile.style.outline = '1px solid red';
        return tile;
    }
});

L.gridLayer.debugCoords = function(opts) {
    return new L.GridLayer.DebugCoords(opts);
};


class NDSLayer{

    constructor(containerMap, ndsInterface, minZoomLevel, maxZoomLevel){
	//
	this.containerMap = containerMap;
	this.ndsInterface = ndsInterface;

	//
	this.minNormalizer = 0.0;
	this.maxNormalizer = 1.0;

	//
	this.colorScale = d3.scaleQuantile().domain([0,1]).range(['#ffffcc','#ffeda0','#fed976','#feb24c','#fd8d3c','#fc4e2a','#e31a1c','#b10026']);
	
	//
	var that = this;
	this.tileLayer = L.gridLayer.canvasCircles({"ndsInterface":ndsInterface,
						    "parent":that,
						    "resolution":5,
						    "opacity":1.0,
						    "myColorScaleDomain":[0,100],
						    "quantileQuery":0.5,
						    "inverseQuantileQuery":100,
						    "queryThreshold":0,
						    "state":"count"});
	this.tileLayer.addTo(this.containerMap);
	//
	// this.debugLayer = L.gridLayer.debugCoords();
	// this.debugLayer.addTo(this.containerMap);
    }

    setMode(newState){
	this.tileLayer.setState(newState);
    }
    
    setOpacity(newValue){
	this.tileLayer.setOpacity(newValue);
    }

    setResolution(newValue){
	this.tileLayer.setResolution(newValue);
    }

    setQuantileMapQuantile(v){
	this.tileLayer.setQuantileMapQuantile(v);
    }

    setInverseQuantileMapQuantile(v){
	this.tileLayer.setInverseQuantileMapQuantile(v);
    }
    
    setColorNormalization(newMin,newMax){
	this.minNormalizer = newMin;
	this.maxNormalizer = newMax;
	this.tileLayer.setExtent([newMin,newMax]);
	this.tileLayer.updateTiles();
    }

    setEnabled(v){
	this.enabled = v;
	if(this.enabled){
	    this.tileLayer.addTo(this.containerMap);
	}
	else{
	    this.tileLayer.removeFrom(this.containerMap);
	}
    }

    repaint(){
	this.tileLayer.redraw();
    }

    setInverseQuantileFilter(minFilter,maxFilter){
	this.tileLayer.setInverseQuantileFilter(minFilter,maxFilter);
    }

    setMyColorScaleDomain(domain){
	this.tileLayer.setMyColorScaleDomain(domain);
	this.repaint();
    }

    setQueryThreshold(v){
	this.tileLayer.setQueryThreshold(v);
    }
    
    renderData(){
	//console.log("Nanocube layer: render");
    }
}
