var shaderSources = {
    "vertex":{

	"modelViewShader":`
    attribute vec4 aVertexPosition;
    uniform mat4   uModelViewMatrix;
    uniform mat4   uProjectionMatrix;
    void main() {
      gl_Position = uProjectionMatrix * uModelViewMatrix * aVertexPosition;
    }
  `,
	"pointShader":`
    attribute vec4 aVertexPosition;
    uniform mat4   uModelViewMatrix;
    uniform mat4   uProjectionMatrix;
    void main() {
gl_PointSize = 10.0;
      gl_Position = uProjectionMatrix * uModelViewMatrix * aVertexPosition;
    }
  `,
	"latLngShader":`
    const float PI = 3.1415926535897932384626433832795;
    const float pi_180 = 0.017453292519943295769236907684886127134428718885417254560;
    const float pi_4   = 12.56637061435917295385057353311801153678867759750042328389;
    attribute vec2 aVertexPosition;
    attribute float aTextureCoordinates;
    uniform mat4   uProjectionMatrix;
    varying float texCoords;

    vec2 latLngToPixel(float latitude, float longitude){
            float sinLatitude = sin(latitude * pi_180);
            float pixelY = (0.5 - log((1.0 + sinLatitude) / (1.0 - sinLatitude)) / (pi_4)) * 256.0;
            float pixelX = ((longitude + 180.0) / 360.0) * 256.0;

            return vec2(pixelX,pixelY);
    }    

void main() {
      gl_PointSize = 10.0;
      texCoords = aTextureCoordinates;
      gl_Position = uProjectionMatrix * vec4(latLngToPixel(aVertexPosition.x, aVertexPosition.y),0.0,1.0);
    } 
	`,

	"latLngPickingShader":`
	const float PI = 3.1415926535897932384626433832795;
	const float pi_180 = 0.017453292519943295769236907684886127134428718885417254560;
	const float pi_4   = 12.56637061435917295385057353311801153678867759750042328389;
	attribute vec2 aVertexPosition;
	attribute float aTextureCoordinates;
	attribute vec3 aColorIndices;
	uniform mat4   uProjectionMatrix;
	varying float texCoords;
	varying vec3 colorIDS;

    vec2 latLngToPixel(float latitude, float longitude){
            float sinLatitude = sin(latitude * pi_180);
            float pixelY = (0.5 - log((1.0 + sinLatitude) / (1.0 - sinLatitude)) / (pi_4)) * 256.0;
            float pixelX = ((longitude + 180.0) / 360.0) * 256.0;
	
            return vec2(pixelX,pixelY);
    }    

void main() {
    gl_PointSize = 10.0;
    texCoords = aTextureCoordinates;
    colorIDS = aColorIndices;
    gl_Position = uProjectionMatrix * vec4(latLngToPixel(aVertexPosition.x, aVertexPosition.y),0.0,1.0);
    } 
  `
	    
    },
    "fragment":{
	"constantColor" : `void main() {
                 gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
                 }
                `,
	
	"uniformColor" : `precision mediump float;    
                          uniform vec4 uPolColor;    
       void main() {
      gl_FragColor = uPolColor;
    }
  `,
	"circlePointConstantColor":`
          precision mediump float;
          void main() {                 
	    float border = 0.001;
            float radius = 0.5;
            vec4 color0 = vec4(0.0, 0.0, 0.0, 0.0);
            vec4 color1 = vec4(0.0,1.0,0.0, 0.2);
	    
            vec2 m = gl_PointCoord.xy - vec2(0.5, 0.5);
            float dist = radius - sqrt(m.x * m.x + m.y * m.y);
            if(dist <= -border){
                discard;
            }
            else if(dist <= 0.0){
                gl_FragColor = mix(color0,color1,(-dist/border));
            }      
            else{
                gl_FragColor = vec4(0.0,1.0,0.0,1.0);
            }           
        }
                `,
	"circlePointTextureColor":`
          precision mediump float;
          varying float texCoords;
          uniform sampler2D uColormapTexture;
          
          void main() {                 
	    float border = 0.001;
            float radius = 0.5;
            vec4 borderColor = vec4(0.5,0.5,0.5,0.5);	    

            vec2 m = gl_PointCoord.xy - vec2(0.5, 0.5);
            float dist = radius - sqrt(m.x * m.x + m.y * m.y);
            if(dist <= -border){
                discard;
            }
            else if(dist <= 0.0){
                gl_FragColor = borderColor;
            }      
            else{
                gl_FragColor = texture2D(uColormapTexture,vec2(texCoords,0.5));
            }           
        }
        `,

	"circlePointPicking":`
        precision mediump float;
        varying float texCoords;
	varying vec3 colorIDS;
	uniform sampler2D uColormapTexture;
	uniform int uRenderPicking;
	uniform int uColorMode;
	
        void main() {
            if(uRenderPicking == 0){
		float border = 0.001;
		float radius = 0.5;
		vec4 borderColor = vec4(0.0,0.0,0.0,1.0);	    

		vec2 m = gl_PointCoord.xy - vec2(0.5, 0.5);
		float dist = radius - sqrt(m.x * m.x + m.y * m.y);
		if(dist <= -border){
		    discard;
		}
		else if(dist <= 0.0){
		    gl_FragColor = borderColor;
		}      
		else{
		    if(uColorMode == 0){
			gl_FragColor = vec4(0.89411764705,0.10196078431,0.10980392156,1.0);
		    }
		    else{
		    gl_FragColor = texture2D(uColormapTexture,vec2(texCoords,0.5));
		    }
		}
	    }
	    else{
		gl_FragColor = vec4(colorIDS,1.0);
	    }
        }
        `,
	
		"textureColor":`
          precision mediump float;
          varying float texCoords;
          uniform float uMinNormalizer;
	  uniform float uMaxNormalizer;
          uniform sampler2D uColormapTexture;
          uniform float uOpacity;
          
          void main() {                 

               //
               float textureCoords = (texCoords - uMinNormalizer)/(uMaxNormalizer - uMinNormalizer);
if(textureCoords < 0.0 || textureCoords > 1.0)
discard;
	       gl_FragColor = vec4(texture2D(uColormapTexture,vec2(textureCoords,0.5)).rgb,uOpacity);
            }           
        
                `
    }
};

 
