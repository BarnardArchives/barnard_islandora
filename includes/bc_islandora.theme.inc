<?php

function theme_bc_islandora_object($vars) {
  $pid = $vars['pid'];
  $mods = _bc_islandora_mods_object($pid);
  $fedora_object = $mods['object'];
  $title = !$vars['title'] ? (isset($mods['mods']) ? $mods['mods']->titleInfo->title->__toString() : '') : $vars['title'];
  $start_page = !$vars['paged'] && !$vars['start_page'] ? 1 : $vars['start_page'];
  $output = '';

  if (!$fedora_object) {
    drupal_not_found();
    return;
  }

  $output .= _bc_islandora_breadcrumb();
  if ($title) {
    $output .= "<h2>{$title}</h2>";
  }
  // Provide links for page, if specified.
  if ($vars['paged'] || $start_page > 1) {
    $output .= '<div id="dl-links">Download: ' . _bc_islandora_dl_links(islandora_object_load($pid . '-' . $start_page)) . '</div>';
  }
  else {
    $output .= '<div id="dl-links">Download: ' . _bc_islandora_dl_links($fedora_object) . '</div>';
  }

  $cmodel = array_shift($fedora_object->relationships->get('info:fedora/fedora-system:def/model#', 'hasModel'));
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  switch ($cmodel['object']['value']) {
    case 'islandora:newspaperIssueCModel':
      $identifiers = array();
      $pages = islandora_paged_content_get_pages($fedora_object);
      foreach ($pages as $pid => $page) {
        $page_metadata = islandora_paged_content_get_page_metadata_from_djatoka($pid);
        if ($start_page > 1 && $page['page'] < $start_page) {
          $identifiers[count($pages)+$page['page']-2] = $page_metadata['identifier'];
        }
        elseif ($start_page == 1) {
          $identifiers[] = $page_metadata['identifier'];
        }
        else {
          $identifiers[$page['page']-2] = $page_metadata['identifier'];
        }
      }
      ksort($identifiers);
      $output .= theme('islandora_openseadragon_viewer', array('uri' => $identifiers, 'fedora_object' => $fedora_object));
    break;
    case 'islandora:newspaperPageCModel':
      // If we got a page, show the issue.
      // TODO but viewer should start with the appropriate page. (or just
      // redirect?)
      $page_number = array_pop(explode('-', $pid));
      $parent_pid = preg_replace('/-' . $page_number . '$/', '', $pid);
      $output = theme('bc_islandora_object', array(
        'pid' => $parent_pid,
        'start_page' => $page_number,
        'paged' => TRUE,
      ));
      /*if ($solr_result = _bc_islandora_solr_query(array('PID:"' . $parent_pid . '"'))) {
        $output .= theme('bc_islandora_object', array(
          'solr_obj' => $solr_result['response']['objects'][0],
          'title' => $title,
          'start_page' => intval($solr_obj['solr_doc']['RELS_EXT_isPageNumber_literal_ms'][0]),
        ));
      }
      else {
        watchdog('bc_islandora', "Didn't get solr result for page :(", 'warning');
      }*/
    break;
    case 'islandora:bookCModel':
      $output .= '<div id="book-viewer">';
      $params = array(
        'object' => $fedora_object,
        'pages' => islandora_paged_content_get_pages($fedora_object),
        'page_progression' => islandora_paged_content_get_page_progression($fedora_object),
      );
      module_load_include('inc', 'islandora', 'includes/solution_packs');
      $viewer = islandora_get_viewer($params, 'islandora_book_viewers');
      $output .= $viewer;
      $output .= '</div>';
    break;
  }

  return $output;
}

function theme_bc_islandora_exhibit($vars) {
  global $base_url;
  $node = isset($vars['node']) ? $vars['node'] : NULL;
  $output = '<div id="exhibition-nav">';
  $output .= '<span class="exhibition-prev"><a href="#">&lt;</a></span>';
  $output .= '<span class="exhibition-next"><a href="#">&gt;</a></span>';
  $output .= '</div>';
  $output .= '<div class="exhibition">';
  if ($node && $node->type == 'exhibition') {
    $exhibition_objects = array();
    foreach (field_get_items('node', $node, 'field_object') as $obj) {
      if (isset($obj['node'])) {
        $pid = array_shift(field_get_items('node', $obj['node'], 'field_fedora_pid'));
        $description = array_shift(field_get_items('node', $obj['node'], 'field_description'));
        $f_o = islandora_object_load($pid['value']);
        if (isset($f_o['JPG'])) {
          $output .= '<div class="exhibition-object">';
          $output .= '<img src="' . url($base_url . '/islandora/object/' . $pid['value'] . '/datastream/JPG/view') . '">';
          $output .=  '<span class="caption" style="display:block">' . $description['value'] . '</span>';
          $output .= '</div>';
        }
      }
    }
  }

  $output .= '</div>';
  return $output;
}

function _bc_islandora_breadcrumb() {
  $breadcrumb = drupal_get_breadcrumb();
  if (array_shift(arg()) == 'view') {
    $type = array_shift(explode('-', arg(1)));
    $path = drupal_get_normal_path($type);
    if ($node = menu_get_item($path)) {
      $breadcrumb[] = l($node['title'], $path);
    }
  }
  return theme_breadcrumb(array('breadcrumb' => $breadcrumb));
}

function _bc_islandora_dl_links($obj, $datastreams = array('PDF', 'JPG')) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  $urls = array();
  foreach ($datastreams as $ds) {
    if (isset($obj[$ds])) {
      $dl_url = islandora_datastream_get_url($obj[$ds], 'download');
      $urls[] = l($ds, $dl_url);
    }
  }
  return implode('<span class="separator">&nbsp;|&nbsp;</span>', $urls);
}
