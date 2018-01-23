class LatLngQuadTreeLayer{

    constructor(){
	//
	this.proj = d3.geoMercator();
	//
	this.minLat =  -85.05113;
	this.maxLat =   85.05113;
	this.minLng = -180.0;
	this.maxLng =  180.0;
	//
	var lowerLeft = this.proj([this.minLng,this.minLat]);
	var topRight  = this.proj([this.maxLng,this.maxLat]);
	this.minX = lowerLeft[0];
	this.minY = lowerLeft[1];
	this.maxX = topRight[0];
	this.maxY = topRight[1];
	//
	this.prefixes = {};
    }
    
    //["0012","0231...",...]
    getPathGeometry(listPath){
	//
	var result = [];
	if(true){
	    //
	    for(var i = 0 ; i < listPath.length ; ++i){
		var path = listPath[i];
		var length = path.length;
		var boundingBox = [[this.minX,this.maxX],[this.minY,this.maxY]];
		//try to find already computed paths	    
		var alreadyComputed = false;
		var pathIndex = length;
		for( ; pathIndex >= 0 ; pathIndex--){
		    var pathPrefix = path.substring(0,pathIndex);
		    if(pathPrefix in this.prefixes){
			//console.log("found prefix",pathPrefix);
			boundingBox = this.prefixes[pathPrefix];
			break;
		    }
		}
		//
		var prefix    = path.substring(0,pathIndex);
		var startPath = path.substring(pathIndex);
		for(var pathIndex = 0; pathIndex < startPath.length ; ++pathIndex){
		    var childIndex = startPath[pathIndex];
		    //subdivide
		    boundingBox = this.subdivide(boundingBox,childIndex);
		    //record
		    prefix += childIndex;
		    this.prefixes[prefix] = boundingBox;
		}
		//
		var lowerLeft = this.proj.invert([boundingBox[0][0],boundingBox[1][0]])
		var topRight  = this.proj.invert([boundingBox[0][1],boundingBox[1][1]])
		//
		result.push([[lowerLeft[0],topRight[0]],[lowerLeft[1],topRight[1]]]);
	    }
	}
	else{
	     //
	    for(var i = 0 ; i < listPath.length ; ++i){
		var path = listPath[i];
		var length = path.length;
		var boundingBox = [[this.minX,this.maxX],[this.minY,this.maxY]];
		for(var pathIndex = 0; pathIndex < length ; ++pathIndex){
		    var childIndex = path[pathIndex];
		    //subdivide
		    boundingBox = this.subdivide(boundingBox,childIndex);
		}
		//
		var lowerLeft = this.proj.invert([boundingBox[0][0],boundingBox[1][0]])
		var topRight  = this.proj.invert([boundingBox[0][1],boundingBox[1][1]])
		//
		result.push([[lowerLeft[0],topRight[0]],[lowerLeft[1],topRight[1]]]);
	    }
	}
	return result;
    }

    subdivide(boundingBox, childIndex){
	var xDomain = boundingBox[0];
	var xCenter = (xDomain[0]+xDomain[1])/2;
	var yDomain = boundingBox[1];
	var yCenter = (yDomain[0]+yDomain[1])/2;
	switch (childIndex) {
	case '0':
	    //lower left
	    return [[xDomain[0],xCenter],[yDomain[0],yCenter]];
	    break;
	case '1':
	    return [[xCenter,xDomain[1]],[yDomain[0],yCenter]];	    
	    break;
	case '2':
	    return [[xDomain[0],xCenter],[yCenter,yDomain[1]]];
	    break;
	case '3':
	    return [[xCenter,xDomain[1]],[yCenter,yDomain[1]]];
	    break;
        default:
            alert('Esse animal irá para Arca de Noé');
            break;
	}
    }
    
}
