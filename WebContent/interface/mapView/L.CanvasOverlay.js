/*
  Generic Canvas Overlay for leaflet.

  ** Requires **
  - Leaflet 1.0.0rc1 or later

  ** Copyright **
  (C) 2014 Stanislav Sumbera
  - http://blog.sumbera.com/2014/04/20/leaflet-canvas/

  ** Maintained by **
  (C) 2015-16 Alexander Schoedon <schoedon@uni-potsdam.de>

  All rights reserved.

  ** Credits **

  Inspired by Stanislav Sumbera's Leaflet Canvas Overlay.
  - http://blog.sumbera.com/2014/04/20/leaflet-canvas/

  Inspired and portions taken from Leaflet Heat.
  - https://github.com/Leaflet/Leaflet.heat

*/

/**
 * Leaflet canvas overlay class
 */
L.CanvasOverlay = L.Layer.extend({

  initialize: function(userDrawFunc, options) {
    this._userDrawFunc = userDrawFunc;
    L.setOptions(this, options);
  },

  params:function(options){
    L.setOptions(this, options);
    return this;
  },

  drawing: function(userDrawFunc) {
    this._userDrawFunc = userDrawFunc;
    return this;
  },

  canvas: function() {
    return this._canvas;
  },

  redraw: function() {
    if (!this._frame) {
      this._frame = L.Util.requestAnimFrame(this._redraw, this);
    }
    return this;
  },

  onAdd: function(map) {
    this._map = map;
    this._canvas = L.DomUtil.create('canvas', 'leaflet-heatmap-layer');

    var size = this._map.getSize();
    this._canvas.width = size.x;
    this._canvas.height = size.y;

    var animated = this._map.options.zoomAnimation && L.Browser.any3d;
    L.DomUtil.addClass(this._canvas, 'leaflet-zoom-'
      + (animated ? 'animated' : 'hide'));

    map._panes.overlayPane.appendChild(this._canvas);

    map.on('moveend', this._reset, this);
    map.on('resize', this._resize, this);

    if (map.options.zoomAnimation && L.Browser.any3d) {
      map.on('zoomanim', this._animateZoom, this);
    }

    this._reset();
  },

  onRemove: function(map) {
    map.getPanes().overlayPane.removeChild(this._canvas);

    map.off('moveend', this._reset, this);
    map.off('resize', this._resize, this);

    if (map.options.zoomAnimation) {
      map.off('zoomanim', this._animateZoom, this);
    }

    this_canvas = null;
  },

  addTo: function(map) {
    map.addLayer(this);
    return this;
  },

  _resize: function(resizeEvent) {
    this._canvas.width = resizeEvent.newSize.x;
    this._canvas.height = resizeEvent.newSize.y;
  },

  _reset: function() {
    var topLeft = this._map.containerPointToLayerPoint([0, 0]);
    L.DomUtil.setPosition(this._canvas, topLeft);
    this._redraw();
  },

  _redraw: function () {
    var size = this._map.getSize();
    var bounds = this._map.getBounds();
    var zoomScale = (size.x * 180) / (20037508.34 * (bounds.getEast()
      - bounds.getWest())); // resolution = 1/zoomScale
    var zoom = this._map.getZoom();

    if (this._userDrawFunc) {
      this._userDrawFunc(this, {
        canvas: this._canvas,
        bounds: bounds,
        size: size,
        zoomScale: zoomScale,
        zoom: zoom,
        options: this.options
      });
    }

    this._frame = null;
  },

    _animateZoom: function (e) {
	//debugger
	var scale = this._map.getZoomScale(e.zoom),
            offset = this._map._latLngBoundsToNewLayerBounds(this._map.getBounds(), e.zoom, e.center).min;
	
        L.DomUtil.setTransform(this._canvas, offset, scale);
  },
});


/**
 * A wrapper function to create a canvas overlay object.
 *
 * @param {String} userDrawFunc the custom draw callback
 * @param {Array} options an array of options for the overlay
 * @return {Object} a canvas overlay object
 */
L.canvasOverlay = function (userDrawFunc, options) {
  return new L.CanvasOverlay(userDrawFunc, options);
};

