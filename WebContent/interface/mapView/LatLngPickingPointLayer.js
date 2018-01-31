class LatLngPickingPointLayer{
    constructor(gl, shaderManager, minZoomLevel, maxZoomLevel){

	//
	this.numTiles = 24;
	this.numPointsPerTile = 100;
	this.numComponentsPerPoint  = 6;
	
	//
	this.glContext     = gl;
	this.dataInfo      = d3.range(this.numTiles).map(function(d){return {"numPoints":0}});
	
	//
	this.vertexBuffer    = new GLBuffer(this.glContext, this.glContext.ARRAY_BUFFER, "float32");
	this.setVertexBufferSize(this.numTiles * this.numPointsPerTile,this.numComponentsPerPoint);
	this.shaderManager   = shaderManager;
	this.dataInitialized = false;
	this.useColor        = false;
	this.enabled         = true;
	this.numIndexComponents = 3;
	this.renderPickingImage = false;
	this.colorMode = 0;
	
	//default color scale
	this.colormapTexture = this.glContext.createTexture();
	this.glContext.bindTexture(this.glContext.TEXTURE_2D, this.colormapTexture);
	// 3x1 pixel 1d texture
	var oneDTextureTexels = new Uint8Array([
	    166,206,227,255,
	    31,120,180,255,
	    178,223,138,255,
	    51,160,44,255,
	    251,154,153,255,
	    227,26,28,255,
	    253,191,111,255,
	    255,127,0,255,
	    0,0,0,255
	]);
	this.glContext.texImage2D(this.glContext.TEXTURE_2D, 0, gl.RGBA, 9, 1, 0, this.glContext.RGBA, this.glContext.UNSIGNED_BYTE, oneDTextureTexels);
    }

    setColorMode(v){
	this.colorMode = v;
    }
    
    setRenderPickingImageFlag(v){
	this.renderPickingImage = v;
    }
    
    indexToUByteArray(index,numComponents){
	var successorIndex = index+1; //avoids the 0 vector
	var result = [];
	for(var i = (numComponents-1) ; i >= 0 ; --i){
	    var d = successorIndex % 256;
	    successorIndex = Math.floor(successorIndex / 256);
	    result.push(d/255.0);
	}
	return result;
    }
    
    uByteArrayToIndex(ubyteArray,numComponents){
	ubyteArray[0] -= 1; //avoids the 0 vector
	var result = 0;
	for(var i = 0 ; i < numComponents ; ++i){
	    result += (ubyteArray[i]*(256**i))
	}
	return result;
    }

    //
    setVertexBufferSize(numElements,numComponents){
	var totalSize = numElements * numComponents;
	var foo = Array(totalSize).fill(0);
	this.vertexBuffer.bind();
	this.vertexBuffer.setData(foo,this.glContext.STREAM_DRAW);
    }

    //
    setSubData(tileIndex,numPoints,vertexArray,colorArray){
	this.glContext.bufferSubData(this.glContext.ARRAY_BUFFER, offset, data); 

	//console.log("LatLngPointLayer setData");
	if(colorArray == undefined){
	    colorArray    = [];
	    this.useColor = false;
	}
	else{
	    this.useColor = true;
	}

	//
	var ubyteArray = [];
	for(var i = 0 ; i < (this.numTiles*this.numPointsPerTile) ; ++i){                     
	    ubyteArray = ubyteArray.concat(this.indexToUByteArray(i,this.numIndexComponents)); 
	}

	//
	this.vertexBuffer.bind();
	var dataArray = vertexArray.concat(colorArray).concat(ubyteArray);
	this.vertexBuffer.setData(dataArray,this.glContext.STREAM_DRAW);
	this.vertexBuffer.release();
    }
    
    //
    setData(dataInfo,vertexArray,colorArray){
	//console.log("LatLngPointLayer setData");
	if(colorArray == undefined){
	    colorArray    = [];
	    this.useColor = false;
	}
	else{
	    this.useColor = true;
	}

	//
	var ubyteArray = [];
	for(var i = 0 ; i < dataInfo.numPoints ; ++i){	                     
	    ubyteArray = ubyteArray.concat(this.indexToUByteArray(i,this.numIndexComponents)); 
	}
	//
	this.dataInfo = dataInfo;
	//
	this.vertexBuffer.bind();
	var dataArray = vertexArray.concat(colorArray).concat(ubyteArray);
	this.vertexBuffer.setData(dataArray,this.glContext.STREAM_DRAW);
	console.log("Point Vertex Buffer", this.vertexBuffer.getSize());
	//
	this.dataInitialized = true;
    }

    setEnabled(v){
	this.enabled = v;
    }

    renderPoints(){
	this.dataInfo.forEach(arraInfo => {
	    this.glContext.drawArrays(this.glContext.POINTS,0,this.dataInfo.numPoints);
	    this.glContext.drawArrays(this.glContext.POINTS,0,this.dataInfo.numPoints);
	});
    }
    
    renderData(){
	//console.log("LatLngPointLayer renderData");

	//
	if(!this.enabled)
	    return;

	//
	const numComponents = 2;
	const type = this.glContext.FLOAT;
	const normalize = false;
	const stride = 0;
	const offset = 0;
	
	//
	var shaderProgram = this.shaderManager.getShader("latLngPickingPointShader");
	shaderProgram.bind();
	this.vertexBuffer.bind();
	//
	this.glContext.vertexAttribPointer(
	    shaderProgram.getAttribLocation('aVertexPosition'),
	    2,//numComponents,
	    this.glContext.FLOAT,//type,
	    false,//normalize,
	    0,//stride,
	    0);//offset);
	this.glContext.enableVertexAttribArray(shaderProgram.getAttribLocation('aVertexPosition'));
	//
	this.glContext.vertexAttribPointer(
	    shaderProgram.getAttribLocation('aTextureCoordinates'),
	    1,//numComponents,
	    this.glContext.FLOAT,//type,
	    false,//normalize,
	    0,//stride,
	    this.dataInfo.numPoints*2*4);//offset);
	this.glContext.enableVertexAttribArray(shaderProgram.getAttribLocation('aTextureCoordinates'));
	//
	this.glContext.vertexAttribPointer(
	    shaderProgram.getAttribLocation('aColorIndices'),
	    3,//numComponents,
	    this.glContext.FLOAT,//type,
	    false,//normalize,
	    0,//stride,
	    this.dataInfo.numPoints*3*4);//offset);
	this.glContext.enableVertexAttribArray(shaderProgram.getAttribLocation('aColorIndices'));
	
	    
	//
	this.glContext.activeTexture(this.glContext.TEXTURE0);
	this.glContext.bindTexture(this.glContext.TEXTURE_2D, this.colormapTexture);
	shaderProgram.setUniform1i('uColormapTexture', 0);
	shaderProgram.setUniform1i('uRenderPicking', (this.renderPickingImage?1:0));
	shaderProgram.setUniform1i('uColorMode', (this.colorMode));
	//
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_MIN_FILTER, this.glContext.NEAREST);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_MAG_FILTER, this.glContext.NEAREST);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_WRAP_S, this.glContext.CLAMP_TO_EDGE);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_WRAP_T, this.glContext.CLAMP_TO_EDGE);

	    
	//
	if(this.dataInitialized && this.dataInfo.numPoints > 0){
	    if(this.renderPickingImage){
		this.glContext.disable(this.glContext.BLEND);
		this.renderPoints()
	    }
	    else{
		this.glContext.enable(this.glContext.BLEND);
		this.glContext.blendFunc(this.glContext.SRC_ALPHA, this.glContext.ONE_MINUES_SRC_ALPHA);
		this.renderPoints();
		this.glContext.disable(this.glContext.BLEND);
	    }
	}

	this.glContext.bindTexture(this.glContext.TEXTURE_2D, null);
	shaderProgram.release();
    }
}
