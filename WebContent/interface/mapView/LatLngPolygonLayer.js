class LatLngPolygonLayer{
    constructor(gl, shaderManager, minZoomLevel, maxZoomLevel){
	//
	this.glContext = gl;
	this.dataInfo = {};
	this.numPoints = 0;
	this.opacity = 1.0;
	this.minNormalizer = 0.0;
	this.maxNormalizer = 1.0;
	//
	this.vertexBuffer = new GLBuffer(this.glContext, this.glContext.ARRAY_BUFFER, "float32");
	this.indexBuffer  = new GLBuffer(this.glContext, this.glContext.ELEMENT_ARRAY_BUFFER, "uint16");
	this.shaderManager = shaderManager;

	//
	this.dataInitialized = false;
	this.enabled         = true;
	//
	//default color scale
	this.colormapTexture = this.glContext.createTexture();
	this.glContext.bindTexture(this.glContext.TEXTURE_2D, this.colormapTexture);
	// 3x1 pixel 1d texture
	var oneDTextureTexels = new Uint8Array([
	    255,247,236,255,
	    254,232,200,255,
	    253,212,158,255,
	    253,187,132,255,
	    252,141,89,255,
	    239,101,72,255,
	    215,48,31,255,
	    179,0,0,255
	]);

	var divergingTextureTexels = new Uint8Array([
	    215,48,39,255,
	    244,109,67,255,
	    253,174,97,255,
	    254,224,144,255,
	    224,243,248,255,
	    171,217,233,255,
	    116,173,209,255,
	    69,117,180,255]);
	
	this.glContext.texImage2D(this.glContext.TEXTURE_2D, 0, gl.RGBA, 8, 1, 0, this.glContext.RGBA, this.glContext.UNSIGNED_BYTE,
				  oneDTextureTexels);
    }

    //
    loadJSONFile(dataFile){
	//TODO
	alert("ERROR LatLngPolygonLayer: Using function loadJASONFile not implemented!");
	return undefined;

    }

    //
    setOpacity(newValue){
	this.opacity = newValue;
    }
    
    //vertex array is a collection of the form [ [v1_x,v1_y], [v2_x,v2_y], ..., [vN_x,vN_y]]
    //index array is a collection of the form [ [i1_t1,i2_t1,i3_t1], [i1_t2,i2_t2,i3_t2], ... , [i1_tM,i2_tM,i3_tM] ]
    //index array is a collection of the form [ [r_t1,g_t1,b_t1], [r_t2,g_t2,b_t2], ... , [r_tM,g_tM,b_tM] ]
    setData(dataInfo,vertexArray,indexArray,colorArray){
	//console.log("LatLngPolygonLayer setData");
	
	//normalize values for colors
	var extent = d3.extent(colorArray);
	this.minNormalizer = extent[0];
	this.maxNormalizer = extent[1];	
	if(this.maxNormalizer == this.minNormalizer)
	    this.maxNormalizer += 1;
	// console.log("===> Normalizer",this.minNormalizer,this.maxNormalizer);
	// console.log(colorArray);
	// var normalizer = (extent[1]==extent[0])?1.0:(extent[1]-extent[0]);
	//colorArray = colorArray.map(function(d){return (d-extent[0])/normalizer});

	//
	this.numPoints = vertexArray.length/2;
	this.dataInfo = dataInfo;

	//
	this.indexBuffer.bind();
	this.indexBuffer.setData(indexArray, this.glContext.DYNAMIC_DRAW);
	console.log("Pol Index Buffer", this.indexBuffer.getSize());

	//
	this.vertexBuffer.bind();
	this.vertexBuffer.setData(vertexArray.concat(colorArray),this.glContext.DYNAMIC_DRAW);
	console.log("Pol Vertex Buffer", this.vertexBuffer.getSize());

	//
	this.dataInitialized = true;
    }

    setColorNormalization(newMin,newMax){
	this.minNormalizer = newMin;
	this.maxNormalizer = newMax;
    }
    
    setEnabled(v){
	this.enabled = v;
    }
    
    renderData(){
	//console.log("LatLngPolygonLayer renderData");
	if(!this.enabled)
	    return;
	//
	const numComponents = 2;
	const type = this.glContext.FLOAT;
	const normalize = false;
	const stride = 0;
	const offset = 0;

	//
	var shaderProgram = this.shaderManager.getShader("latLngShader");
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
	    this.numPoints*2*4);//offset);
	this.glContext.enableVertexAttribArray(shaderProgram.getAttribLocation('aTextureCoordinates'));

	//
	this.glContext.activeTexture(this.glContext.TEXTURE0);
	this.glContext.bindTexture(this.glContext.TEXTURE_2D, this.colormapTexture);
	shaderProgram.setUniform1i('uColormapTexture', 0);
	shaderProgram.setUniform1f('uOpacity',this.opacity);
	//
	shaderProgram.setUniform1f('uMinNormalizer',this.minNormalizer);
	shaderProgram.setUniform1f('uMaxNormalizer',this.maxNormalizer);
	//
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_MIN_FILTER, this.glContext.NEAREST);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_MAG_FILTER, this.glContext.NEAREST);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_WRAP_S, this.glContext.CLAMP_TO_EDGE);
	this.glContext.texParameteri(this.glContext.TEXTURE_2D, this.glContext.TEXTURE_WRAP_T, this.glContext.CLAMP_TO_EDGE);

	//
	this.glContext.enable(this.glContext.BLEND);
	this.glContext.blendFunc(this.glContext.SRC_ALPHA, this.glContext.DST_ALPHA);
	//this.glContext.blendFunc(this.glContext.GL_SRC_ALPHA, this.glContext.GL_ZERO);//(GL_DST_COLOR, GL_ZERO)
	//
	if(true){//this.dataInitialized){
	    this.indexBuffer.bind();

	    for(var key in this.dataInfo){
	     	var elt = this.dataInfo[key];
		//TODO: hardcoding 2* of the unsigned_short index type
		this.glContext.drawElements(
		    this.glContext.TRIANGLES,
		    elt.numVerticesToBeRendered,
		    this.glContext.UNSIGNED_SHORT,
		    2*elt.minArrayIndex);
	    }
	}
	else{
	    //debug purposes
	    this.glContext.drawElements(
		this.glContext.TRIANGLES,
		3,
		this.glContext.UNSIGNED_SHORT,
		3*2);	    
	}

	this.glContext.bindTexture(this.glContext.TEXTURE_2D, null);
	this.glContext.disable(this.glContext.BLEND);
	shaderProgram.release();
    }
}
