function toNanocubeCoords(coords){
    var latlng             = tile_to_degree(coords,coords.z,false);
    var nanocubeTileCoords = degree_to_tile(latlng,coords.z,true);
    //return z + "," + nanocubeTileCoords.x + "," + nanocubeTileCoords.y;
    return {"x":nanocubeTileCoords.x, "y": nanocubeTileCoords.y, "z": coords.z};
}

L.GridLayer.CanvasCircles = L.GridLayer.extend({
    myColorScale:function(count){
	var opacity = this.options.opacity
	var lc = Math.log(count + 1) / Math.log(100);

        var r = toString(Math.floor(255 * Math.min(1, lc)),16);
	if(r.length == 1)
	    r = "0" + r;
	
        var g = toString(Math.floor(255 * Math.min(1, Math.max(0, lc - 1))),16);
	if(g.length == 1)
	    g = "0" + g;

        var b = Math.floor(255 * Math.min(1, Math.max(0, lc - 2)));
	if(b.length == 1)
	    b = "0" + b;
	var a = toString(Math.floor(255 * opacity),16);
	if(a.length == 1)
	    a = "0" + a;
	
        return "#" + r + g + b + a;
    },
    colorTile: function(tile, coords){
	//
	var layer = this;
        var ctx = tile.getContext('2d');

	//
        var tileSize = this.getTileSize();
	var resolution = this.options.resolution;
	var numPixels = 2**resolution;
	var pixelWidth  = tileSize.x / numPixels;
	var pixelHeight = tileSize.y / numPixels;		
	
	tile.data.forEach(function(pixel){
	    var pixelInLocalCoords = [pixel[0]-coords[0],pixel[1]-coords[1]];
	    console.log(pixel[2]);
	    var rgba = layer.myColorScale(pixel[2]);
	    ctx.fillStyle = rgba;
	    ctx.fillRect(pixelInLocalCoords[0]*pixelWidth, pixelInLocalCoords[1]*pixelHeight , pixelWidth, pixelHeight);
	});
    },
    createTile: function (coords,done) {
	var layer = this;
        var tile = document.createElement('canvas');
	tile.data = undefined;
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
	var query = new NDSQuery(datasetName,"count",spatialDimension,function(result,myQ){
	    tile.data = result;
	    layer.colorTile(tile,tileInTotalResolution);	    	    
            done(null, tile);	// Syntax is 'done(error, tile)'
	});
	query.addConstraint("tile",spatialDimension,{"x":coords.x,"y":coords.y,"z":coords.z,"resolution":resolution});
	query.tile = [tileInTotalResolution[0],tileInTotalResolution[1],totalResolution];
	ndsInterface.query(query);
	
        return tile;
    	
    }
});

L.gridLayer.canvasCircles = function(opts) {
    return new L.GridLayer.CanvasCircles(opts);
};


L.GridLayer.CanvasLayer = L.GridLayer.extend({
    createTile: function (coords, done) {
	if(true){
	    var tile = document.createElement('canvas');

            var tileSize = this.getTileSize();
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
	this.tileLayer = L.gridLayer.canvasCircles({"ndsInterface":ndsInterface, "parent":that, "resolution":5,"opacity":1.0});
	this.tileLayer.addTo(this.containerMap);

	//
	// this.debugLayer = L.gridLayer.debugCoords();
	// this.debugLayer.addTo(this.containerMap);
    }
    
    setOpacity(newValue){
	this.tileLayer.setOpacity(newValue);
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
    
    renderData(){
	//console.log("Nanocube layer: render");
    }
}
