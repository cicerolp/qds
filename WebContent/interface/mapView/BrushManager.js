class BrushManager {
    
    constructor(mapContainer){
	this.nextID = 0;
	this.brushes = [];
	this.mapContainer = mapContainer;
    }

    newBrush(){
	var bounds = [[0, 0], [0,0]];
	var myColor = d3.schemeSet1[1];//this.nextID%d3.schemeSet1.length];
	var newBrush = L.rectangle(bounds, {color: myColor, weight: 1});
	newBrush.internalID = this.nextID;
	this.nextID += 1;
	this.brushes.push(newBrush);
	newBrush.addTo(this.mapContainer);
	return newBrush;
    }

    getFirstThatContains(latlng){
	var numBrushes = this.brushes.length;
	for(var i = numBrushes-1; i >= 0 ; --i){
	    var d = this.brushes[i];
	    if(d.getBounds().contains(latlng)){
		return d;
	    }
	}
	   
	return undefined;
    }

    getSelectionBounds(){	
	return this.brushes.map(
	    function(d){
		var coords = d.toGeoJSON().geometry.coordinates[0];
		var flattenCoordinates = [];
		coords.forEach(function(d){
		    flattenCoordinates.push(d[1]);
		    flattenCoordinates.push(d[0]);
		});

		//
		return {"id":d.internalID,"color":d.options.color,"geometry":flattenCoordinates};
	    }
	);
    }
    
    remove(brushID){
	var removed = false;
	var index = undefined;
	var numBrushes = this.brushes.length;
	for(var i = 0; i < numBrushes ; ++i){
	    var d = this.brushes[i];
	    if(d.internalID == brushID){
		index = i;
		d.removeFrom(this.mapContainer);
		break;
		
	    }
	}

	if(index != undefined){
	    this.brushes.splice(index,1);
	    removed = true;
	}	

	return removed;
    }

    getNumBrushes(){
	return this.brushes.length;
    }
}
