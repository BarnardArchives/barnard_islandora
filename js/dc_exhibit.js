(function($) {
  Drupal.behaviors.dc_exhibit = {
    show_first: function($selector) {
      $selector.each(function() {
        if (!$(this).hasClass('first')) {
          $(this).hide();
        }
        else {
          $(this).addClass('active');
        }
      });
    },
    show_slide: function($slide) {
      $slide.show().addClass('active');
      $slide.find('.large-image:first').show().addClass('active');
      $slide.find('.ex-thumbnails img:first').addClass('active');
    },
    hide_slide: function($slide) {
      $slide.hide().removeClass('active');
      $slide.find('.large-image.active').hide().removeClass('active');
      $slide.find('.ex-thumbnails img.active').removeClass('active');
    },
    toggle_slide: function($from_slide, $to_slide) {
      var self = this;
      self.hide_slide($from_slide);
      self.show_slide($to_slide);
    },
    activate_theme: function(theme) {
      var self = this;
      var $theme = $('#ex-themes .theme[data-theme="' + theme + '"]');
      var $active_theme = $('#ex-themes .theme.active');
      var active_theme = $active_theme.innerHTML;
      console.log(theme);
      console.log(active_theme);
      if (active_theme != theme) {
        $active_theme.removeClass('active');
        $theme.addClass('active');
      }
    },
    listeners: function() {
      var self = this;
      // theme click
      $('#ex-themes .theme').click(function(event) {
        event.preventDefault();
        var theme = this.innerHTML;
        var $theme_first = $('.ex-obj[data-theme="' + theme + '"]:first');
        var $active_obj = $('.ex-obj.active');
        if (!$theme_first.hasClass('active')) {
          self.toggle_slide($active_obj, $theme_first);
          // $theme_first.addClass('active').show();
          // $theme_first.find('.ex-images').addClass('active').show();
          // $($theme_first.find('.ex-images')[0]).find('.large-image.first').show();
        }
        if (!$(this).hasClass('active')) {
          $(this).parent().find('.active').removeClass('active');
          $(this).addClass('active');
        }
      });
      // thumbnail click
      $('.ex-thumbnails a').click(function(event) {
        event.preventDefault();
        // The "id" of the clicked thumbnail.
        var active_img_id = $(this).find('img').attr('id').split('tn')[1];
        // The current "active" image: hide it.
        var $active_img = $(this).parent().parent().find('.large-image.active');
        $active_img.hide().removeClass('active');
        // The image corresponding to the clicked thumbnail: show it.
        var $clicked_img = $(this).parent().parent().find('#' + active_img_id);
        $clicked_img.show().addClass('active');
        // Toggle "active" thumbnails.
        $(this).parent().find('img.active').removeClass('active');
        $(this).find('img').addClass('active');
      });
      // nav click
      $('#ex-nav a').click(function(event) {
        event.preventDefault();
        var $active_slide = $('.ex-obj.active'),
          $active_obj_img = $active_slide.find('.large-image.active'),
          $active_tn = $active_obj_img.find('.ex-thumbnails img.active');
        var active_slide_index = parseInt($active_slide.attr('id').split('slide')[1]);
        var next_slide_index = active_slide_index + 1,
          prev_slide_index = active_slide_index - 1;
        var $next_slide = $('.ex-obj#slide' + next_slide_index),
          $prev_slide = $('.ex-obj#slide' + prev_slide_index),
          $first_slide = $('.ex-obj.first'),
          $last_slide = $('.ex-obj:last');
        var $slide_to;
        switch ($(this).attr('id')) {
          case 'next':
            if ($next_slide.length) {
              $slide_to = $next_slide;
            }
            else {
              $slide_to = $first_slide;
            }
            break;
          case 'prev':
            if (prev_slide_index > 0) {
              $slide_to = $prev_slide;
            }
            else {
              $slide_to = $last_slide;
            }
            break;
        }
        self.toggle_slide($active_slide, $slide_to);
        self.activate_theme($slide_to.attr('data-theme'));
      });
    },
    attach: function(context, settings) {
      console.log(context);
      var self = this;
      self.listeners();
      $('.ex-images').each(function() {
        self.show_first($(this).find('.large-image'));
      });
      self.show_first($('.ex-obj'));
      $('.ex-thumbnails').each(function() {
        var active_img = $('.ex-images .large-image.first').attr('id');
        $('.ex-thumbnails img#tn' + active_img).addClass('active');
      });
      self.activate_theme($('.ex-obj.active').attr('data-theme'));
    }
  }  
}) (jQuery);
