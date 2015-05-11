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
      var $theme = $('#ex-themes .theme[data-theme="' + theme + '"]');
      var $active_theme = $('#ex-themes .theme.active');
      var active_theme = $active_theme.innerHTML;
      if (active_theme != theme) {
        $active_theme.removeClass('active');
        $theme.addClass('active');
      }
    },
    pager_click: function(pageNumber, self) {
      var $active_slide = $('.ex-obj.active');
      var $slide_to = $('.ex-obj#slide' + pageNumber);
      self.toggle_slide($active_slide, $slide_to);
      self.activate_theme($slide_to.attr('data-theme'));
    },
    show_hash: function(hash) {
      var self = this;
      var page = parseInt(hash.split('#page-')[1]);
      self.pager_click(page, {}, self);
      // $('#ex-nav').pagination('selectPage', page);
    },
    paginate: function() {
      var self = this;
      var slide_count = $('.ex-obj').length;
      /*$('#ex-nav').pagination({
        items: slide_count,
        itemsOnPage: 1,
        onPageClick: function(pageNumber, event) {
          self.pager_click(pageNumber, event, self);
        },
        prevText: '&larr;',
        nextText: '&rarr;'
      });*/
    },
    listeners: function() {
      var self = this;
      // Theme click.
      $('#ex-themes .theme').click(function(event) {
        event.preventDefault();
        var theme = this.innerHTML;
        var $theme_first = $('.ex-obj[data-theme="' + theme + '"]:first');
        var $active_obj = $('.ex-obj.active');
        if (!$theme_first.hasClass('active')) {
          self.toggle_slide($active_obj, $theme_first);
        }
        if (!$(this).hasClass('active')) {
          $(this).parent().find('.active').removeClass('active');
          $(this).addClass('active');
        }
        // Activate correct pager item.
        // $('#ex-nav').pagination('selectPage', parseInt($theme_first.attr('id').split('slide')[1]))
      });
      // Thumbnail click.
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
      // Nav click: prev.
      $('#ex-nav a#prev').click(function(event) {
        event.preventDefault();
        var page = parseInt($('.ex-obj.active').attr('id').split('slide')[1]);
        var page_count = $('.ex-obj').length;
        // If this is the first page, show the last page. Otherwise, show prev.
        if (page == 1) {
          self.pager_click(page_count, self);
        }
        else {
          self.pager_click(page-1, self);
        }
      });
      // Nav click: next.
      $('#ex-nav a#next').click(function(event) {
        event.preventDefault();
        var page = parseInt($('.ex-obj.active').attr('id').split('slide')[1]);
        var page_count = $('.ex-obj').length;
        // If this is last page, show first page. Otherwise, show next.
        if (page == page_count) {
          self.pager_click(1, self);
        }
        else {
          self.pager_click(page+1, self);
        }
      });
    },
    attach: function(context, settings) {
      var self = this;
      // self.paginate();
      $('.ex-images').each(function() {
        self.show_first($(this).find('.large-image'));
      });
      self.show_first($('.ex-obj'));
      $('.ex-thumbnails').each(function() {
        var active_img = $('.ex-images .large-image.first').attr('id');
        $('.ex-thumbnails img#tn' + active_img).addClass('active');
      });
      // TODO
      if (location.hash.length > 0) {
        // $('#ex-nav').pagination('selectPage', parseInt(location.hash.split('#page-')[1]));
      }
      self.activate_theme($('.ex-obj.active').attr('data-theme'));
      self.listeners();
    }
  }  
}) (jQuery);