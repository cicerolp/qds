class LatLngPointLayer{
    constructor(gl, shaderManager, minZoomLevel, maxZoomLevel){

	//
	this.glContext     = gl;
	this.dataInfo      = {};
	
	//
	this.vertexBuffer    = new GLBuffer(this.glContext, this.glContext.ARRAY_BUFFER, "float32");
	this.shaderManager   = shaderManager;
	this.dataInitialized = false;
	this.useColor        = false;
	this.enabled         = true;
	
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
	]);
	this.glContext.texImage2D(this.glContext.TEXTURE_2D, 0, gl.RGBA, 8, 1, 0, this.glContext.RGBA, this.glContext.UNSIGNED_BYTE,
				  oneDTextureTexels);
    }

    //
    setColors(colors,colorCoefs){
	//TODO???
    }
    
    //
    loadJSONFile(dataFile){
	//TODO
	alert("ERROR Point Layer: Using function loadJASONFile not implemented!");
	return undefined;
    }
    
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
	this.dataInfo = dataInfo;
	//
	this.vertexBuffer.bind();
	this.vertexBuffer.setData(vertexArray.concat(colorArray),this.glContext.STATIC_DRAW);
	console.log("Point Vertex Buffer", this.vertexBuffer.getSize());
	//
	this.dataInitialized = true;
    }

    setEnabled(v){
	this.enabled = v;
    }
    
    renderData(){
	//console.log("LatLngPointLayer renderData");

	//
	if(!this.enabled || !this.dataInitialized)
	    return;

	//
	const numComponents = 2;
	const type = this.glContext.FLOAT;
	const normalize = false;
	const stride = 0;
	const offset = 0;
	
	//
	var shaderProgram = this.shaderManager.getShader("latLngPointShader");
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
	if(this.useColor){
	    this.glContext.vertexAttribPointer(
		shaderProgram.getAttribLocation('aTextureCoordinates'),
		1,//numComponents,
		this.glContext.FLOAT,//type,
		false,//normalize,
		0,//stride,
		this.dataInfo.numPoints*2*4);//offset);
	    this.glContext.enableVertexAttribArray(shaderProgram.getAttribLocation('aTextureCoordinates'));
	}
	    
	//
	this.glContext.activeTexture(this.glContext.TEXTURE0);
	this.glContext.bindTexture(this.glContext.TEXTURE_2D, this.colormapTexture);
	shaderProgram.setUniform1i('uColormapTexture', 0);
	//
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_MIN_FILTER, this.glContext.NEAREST);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_MAG_FILTER, this.glContext.NEAREST);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_WRAP_S, this.glContext.CLAMP_TO_EDGE);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_WRAP_T, this.glContext.CLAMP_TO_EDGE);

	    
	//
	if(this.dataInitialized && this.dataInfo.numPoints > 0){
	    this.glContext.enable(this.glContext.BLEND);
	    this.glContext.blendFunc(this.glContext.SRC_ALPHA, this.glContext.ONE_MINUES_SRC_ALPHA);
	    this.glContext.drawArrays(this.glContext.POINTS,0,this.dataInfo.numPoints);
	    this.glContext.disable(this.glContext.BLEND);
	}

	this.glContext.bindTexture(this.glContext.TEXTURE_2D, null);
	shaderProgram.release();
    }
}
