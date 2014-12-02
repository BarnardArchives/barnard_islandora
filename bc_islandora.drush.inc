<?php

require_once 'includes/bc_islandora.drush_helpers.inc';

function bc_islandora_drush_command() {
  return array(
    'bc_islandora_xml_export' => array(
      'description' => 'Exports an FOXML representation of the given object.',
      'arguments' => array(
        'pid' => 'The pid of the object to export.',
      ),
      'examples' => array(
        'drush -u 1 --uri=http://localhost xml_export islandora:37',
      ),
      'aliases' => array('bcixe'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    ),
    'bc_islandora_purge_object' => array(
      'description' => 'Purges an object from the repository.',
      'arguments' => array(
        'pids' => 'A comma-separated list of unique identifiers.',
      ),
      'aliases' => array('bcipo'),
    ),
    'bc_islandora_ingest_newspapers' => array(
      'description' => "Creates the parent issue object and uploads the pages as child objects.  
        Requires the Drupal7 version of Islandora with tuque",
      'arguments' => array(
        'query_file' => 'The path to a CSV file that contains the information about the newspaper pages',
      ),
      'options' => array(
        'derivatives' => 'Whether to create derivatives.',
        'overwrite-existing' => 'Whether to overwrite existing objects.',
        // TODO move this
        'purge' => 'Whether to purge instead of ingesting.',
      ),
      'examples' => array(
        'drush -u 1 --uri=http://localhost ingest_newspapers /path/to/pages.csv',
      ),
      'aliases' => array('bciin'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN, // we can pass in users id on the command line using drush -u.
    ),
    'bc_islandora_ingest_books' => array(
      'description' => 'Ingests books.',
      'arguments' => array(
        'query_file' => 'A csv file containing information about the books to be ingested.'
      ),
      'options' => array(
        'derivatives' => 'Whether to create derivatives.',
        'overwrite-existing' => 'Whether to overwrite existing objects.',
        // TODO move this
        'purge' => 'Whether to purge instead of ingesting.',
        'year' => 'Whether to import a specific year\'s book.',
      ),
      'examples' => array(
        'drush -u 1 --uri=http://localhost bciib /path/to/pages.csv',
      ),
      'aliases' => array('bciib'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN, // we can pass in users id on the command line using drush -u.
    ),
    'bc_islandora_generate_newspaper_csv' => array(
      'description' => 'Generates csv files for ingesting newspapers.',
      'arguments' => array(
        'directory' => 'The directory containing image files.',
      ),
      'options' => array(
        'year' => 'The year for which to generate csv.',
        'month' => 'The month for which to generate csv.',
      ),
      'examples' => array(
        'drush generate_newspaper_csv /vagrant/bulletin',
      ),
      'aliases' => array('bcignc'),
    ),
    'bc_islandora_fix_metadata' => array(
      'arguments' => array(
        'type' => 'The type of record to fix.',
      ),
      'examples' => array(
        'drush bcifm newspaper',
      ),
      'aliases' => array('bcifm'),
    ),
    'bc_islandora_generate_url_aliases' => array(
      'examples' => array(
        'drush -u 1 bcigua',
      ),
      'aliases' => array('bcigua'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN, // we can pass in users id on the command line using drush -u.
    ),
    'bc_islandora_replace_ocr' => array(
      'arguments' => array(
        'query_file' => 'The path to a CSV file that contains the information about the newspaper pages',
      ),
      'examples' => array(
        'drush -u 1 bciro',
      ),
      'aliases' => array('bciro'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    ),
  );
}

function drush_bc_islandora_xml_export($pid) {
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $foxml = $repository->api->m->export($pid, array(
    'format' => 'info:fedora/fedora-system:FOXML-1.1',
    'context' => 'migrate',
    'encoding' => 'UTF-8',
  ));
  var_dump($foxml);
}

function drush_bc_islandora_ingest_newspapers($query_file) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');

  ini_set("auto_detect_line_endings", "1");

  $path_name = dirname($query_file);
  $fh = fopen($query_file, 'r');
  $line_number = 0;
  $issues = array();

  while (($data = fgetcsv($fh)) !== FALSE) {
    $line_number++;
    if ($line_number > 1) {
      $type = $data[4];
      if ($type == 'issue') {
        $issue_pid = 'bc:bulletin-' . date('Ymd', strtotime($data[1]));
  
        if (drush_get_option('purge')) {
          drush_bc_islandora_purge_object($issue_pid);
        }
        elseif (!islandora_object_load($issue_pid)) {
          try {
            $issue_pid = create_issue_object($issue_pid, $data);
          }
          catch (Exception $e) {
            drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
          }
        }
        elseif (drush_get_option('overwrite-existing')) {
          try {
            drush_bc_islandora_purge_object($issue_pid);
            create_issue_object($issue_pid, $data);
          }
          catch (Exception $e) {
            drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
          }
        }
        else {
          drush_log(dt('Issue exists at pid = @pid; skipping...', array('@pid' => $issue_pid)), 'warning');
        }
        $issues[] = $issue_pid;
      }
      elseif ($type == 'page') {
        $page_pid = 'bc:bulletin-' . date('Ymd', strtotime($data[1])) . '-' . $data[3];
        
        if (drush_get_option('purge')) {
          drush_bc_islandora_purge_object($issue_pid);
        }
        elseif (!islandora_object_load($page_pid)) {
          try {
            create_page_object($path_name, $data, $issue_pid, $page_pid, 'newspaper');
          }
          catch (Exception $e) {
            drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
          }
        }
        elseif (drush_get_option('purge')) {
          drush_bc_islandora_purge_object($page_pid);
        }
        elseif (drush_get_option('overwrite-existing')) {
          try {
            drush_bc_islandora_purge_object($page_pid);
            create_page_object($path_name, $data, $issue_pid, $page_pid, 'newspaper');
          }
          catch (Exception $e) {
            drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
          }
        }
        else {
          drush_log(dt('Page exists at pid = @pid; skipping...', array('@pid' => $page_pid)), 'warning');
        }
      }
    }
  }    

  // Create derivatives.
  if (drush_get_option('derivatives')) {
    _bc_islandora_create_derivatives($issues);
  }
}

function drush_bc_islandora_ingest_books($query_file) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');

  ini_set("auto_detect_line_endings", "1");
  
  $fh = fopen($query_file, 'r');
  $year = drush_get_option('year');
  $books = array();

  while (($data = fgetcsv($fh)) !== FALSE) {
    //$csv_year = array_pop(explode(' ', $data[1]));
    $csv_year = $data[3];
    $pid_year = array_pop(explode(' ', $data[1]));
    if ($year && $year != $csv_year) {
      continue;
    }
    $book_pid = 'bc:yearbook-' . $pid_year;
    $pages = $data[12];
    if (drush_get_option('purge')) {
      drush_bc_islandora_purge_object($book_pid);
    }
    elseif (!islandora_object_load($book_pid)) {
      try {
        print "Got book pid: $book_pid\n";
        $book_pid = create_book_object($book_pid, $data);
      }
      catch (Exception $e) {
        drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
      }
    }
    elseif (drush_get_option('overwrite-existing')) {
      try {
        drush_bc_islandora_purge_object($book_pid);
        create_book_object($book_pid, $data);
      }
      catch (Exception $e) {
        drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
      }
    }
    else {
      drush_log(dt('Issue exists at pid = @pid; skipping...', array('@pid' => $book_pid)), 'warning');
      continue;
    }
    $books[] = $book_pid;

    // Create each page.
    $file_path = dirname($query_file) . '/' . $csv_year;
    if (file_exists($file_path)) {
      $files = explode("\n", `ls $file_path/*.jp2`);
      foreach ($files as $fn) {
        if (!empty($fn)) {
          $page_number = preg_replace('/^0+/', '', preg_replace('/\.jp2$/', '', array_pop(explode('_', $fn))));
          $data[] = $page_number;
          // var_export($page_number);
          $page_pid = $book_pid . '-' . $page_number;
          if (drush_get_option('overwrite-existing') || drush_get_option('purge')) {
            try {
              drush_bc_islandora_purge_object($page_pid);
            }
            catch (Exception $e) {
              drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
            }
          }
          if (!drush_get_option('purge')) {
            create_page_object($fn, $data, $book_pid, $page_pid, 'book'); 
          }
        }
      }
    }
  
    // Create derivatives.
    if (drush_get_option('derivatives')) {
      _bc_islandora_create_derivatives($books);
    }
  }
}

function drush_bc_islandora_purge_object($pids) {
  $tuque = new IslandoraTuque();
  foreach (explode(',', $pids) as $pid) {
    drush_log(dt('Purging object at pid = @pid...', array('@pid' => $pid)), 'ok');
    try {
      $tuque->repository->purgeObject($pid);
    }
    catch (Exception $e) {
      drush_log('Caught exception: @e', array('@e' => $e->getMessage()), 'warning');
    }
  }
}

function drush_bc_islandora_generate_newspaper_csv($directory = '/home/BCIT/dsavage/bulletin') {
  $year = drush_get_option('year');
  $month = drush_get_option('month');
  $filename = $year && $month ? "$year-$month" : ($year ? "$year" : '');
  $filename .= 'pages.csv';
  $csv = fopen("$directory/$filename", 'w');
  $lines = array(
    array("title", "date", "filename", "page number", "type", "extent"),
  );
  $issues = array();

  foreach (scandir($directory) as $file) {
    if (mime_content_type("$directory/$file") == 'image/tiff') {
      $ts = strtotime(array_shift(explode('-', str_replace('BarnardBulletin-', '', $file))));
      $day = date('d', $ts);
      if ((!$month || ($month && date('m', $ts) == $month)) && (!$year || ($year && date('Y', $ts) == $year))) {
        $extent_cmd = 'ls ' . $directory . '/BarnardBulletin-' . date('Ymd', $ts) . '*.tiff | wc -l';
        $extent = trim(`$extent_cmd`);
        $page = preg_replace('/^0+/', '', str_replace('.tiff', '', end(explode('-', $file))));
        $date_title = date('F j, Y', $ts);
        if ($page == 1) {
          $lines[] = array("Barnard Bulletin, " . $date_title, $date_title, '', '', 'issue', $extent);
        }
        $lines[] = array($date_title, $date_title, $file, $page, 'page', $extent);
      }
    }
  }
  foreach ($lines as $line) {
    fputcsv($csv, $line);
  }
  fclose($csv);
}

function drush_bc_islandora_fix_metadata($type, $collection_pid = '') {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_newspaper', 'includes/utilities');

  $tuque = islandora_get_tuque_connection();

  switch($type) {
    case 'newspaper':
      $collection_pid = variable_get('bc_islandora_newspaper_pid', 'islandora:74');
      $paper = islandora_object_load($collection_pid);
      $compound = islandora_newspaper_get_issues($paper);
      $genre = $type;
      $enctype = 'w3cdtf';
    break;
    case 'yearbook':
      $collection_pid = variable_get('bc_islandora_yearbook_pid', 'islandora:bookCollection');
      $compound = islandora_paged_content_get_pages(islandora_object_load($collection_pid));
      $genre = $type;
      $enctype = 'iso8601';
    break;
  }

  $pages = array();
  $new_mods = array();
  foreach ($compound as $pid => $o) {
    $parent_object = islandora_object_load($pid);
    // Parent object, newspaper or book.
    if (isset($parent_object['MODS']) && $parent_mods_xml = simplexml_load_string($parent_object['MODS']->getContent(NULL))) {
      $id_old = (string) $parent_mods_xml->relatedItem->identifier;
      unset($parent_mods_xml->relatedItem->identifier);
      $parent_mods_xml->relatedItem->addChild('identifier', str_replace('.', '-', $id_old))->addAttribute('type', 'local');
    }
    // Parent object, newspaper only.
    if ($type == 'newspaper' && $parent_mods_xml) {
      $date = (string) $parent_mods_xml->originInfo->dateIssued;
      $new_date = date('Y-m-d', strtotime($date));
      if (!isset($parent_mods_xml->genre)) {
        $parent_mods_xml->addChild('genre', $genre)->addAttribute('authority', 'marcgt');
      }
      if (!isset($parent_mods_xml->originInfo->dateIssued)) {
        $parent_mods_xml->originInfo->addChild('dateIssued', $new_date)->addAttribute('encoding', $enctype);
      }
      if (!isset($parent_mods_xml->originInfo->dateCreated)) {
        $parent_mods_xml->originInfo->addChild('dateCreated', $new_date)->addAttribute('encoding', $enctype);
      }
    }
    // Parent object, book only.
    elseif ($type == 'yearbook' && $parent_mods_xml) {
      unset($parent_mods_xml->originInfo->dateCreated);
      $parent_mods_xml->originInfo->addChild('dateCreated', (string) $parent_mods_xml->originInfo->dateIssued)->addAttribute('encoding', $enctype);
    }
    $new_mods[$pid] = $parent_mods_xml->asXML();
    // Child (page) objects.
    foreach (islandora_paged_content_get_pages(islandora_object_load($pid)) as $child_pid => $page) {
      $parent_date = (string) $parent_mods_xml->originInfo->dateIssued;
      $child_object = islandora_object_load($page['pid']);
      if (isset($child_object['MODS']) && $child_mods_xml = simplexml_load_string($child_object['MODS']->getContent(NULL))) {
        // identifier (book page)
        $id_top = (string) $child_mods_xml->identifier;
        if (($type == 'yearbook') && (strpos($id_top, $parent_date) === FALSE)) {
          unset($child_mods_xml->identifier);
          $page_number = str_pad(array_pop(explode(' ', (string) $child_mods_xml->titleInfo->title)), 3, '0', STR_PAD_LEFT);
          $id_top_parts = explode('_', $id_top);
          $id_top_new = implode('_', array($id_top_parts[0], $parent_date, $page_number));
          $child_mods_xml->addChild('identifier', $id_top_new)->addAttribute('type', 'local');
        }
        // relatedItem identifier
        $id_old = (string) $child_mods_xml->relatedItem->identifier;
        if (strpos($id_old, '.') !== FALSE) {
          unset($child_mods_xml->relatedItem->identifier);
          $child_mods_xml->relatedItem->addChild('identifier', str_replace('.', '-', $id_old))->addAttribute('type', 'local');
        }
        // genre
        if (!isset($child_mods_xml->genre)) {
          $child_mods_xml->addChild('genre', $genre)->addAttribute('authority', 'marcgt');
        }
        // originInfo
        if (!isset($child_mods_xml->originInfo)) {
          $child_mods_xml->addChild('originInfo')->addChild('dateIssued', $parent_date)->addAttribute('encoding', $enctype);
        }
        else {
          // dateIssued
          unset($child_mods_xml->originInfo->dateIssued);
          $child_mods_xml->originInfo->addChild('dateIssued', $parent_date)->addAttribute('encoding', $enctype);
          // dateCreated
          unset($child_mods_xml->originInfo->dateCreated);
          $child_mods_xml->originInfo->addChild('dateCreated', $parent_date)->addAttribute('encoding', $enctype);
        }
        $new_mods[$child_pid] = $child_mods_xml->asXML();
      }
    }
  }
  // purge the existing MODS datastreams.
  foreach ($new_mods as $pid => $mods) {
    drush_log(dt('Doing object (purge): @pid', array('@pid' => $pid)), 'ok');
    $object = islandora_object_load($pid);
    $object->purgeDatastream('MODS');
  }
  // then create new MODS datastreams.
  foreach ($new_mods as $pid => $mods) {
    drush_log(dt('Doing object (add): @pid', array('@pid' => $pid)), 'ok');
    $new_object = islandora_object_load($pid);
    $new_ds = $new_object->constructDatastream('MODS');
    $new_ds->label = 'MODS';
    $new_ds->setContentFromString($mods);
    $new_object->ingestDatastream($new_ds);
  }
}

function _bc_islandora_create_derivatives($objects) {
  module_load_include('inc', 'bc_islandora', 'includes/bc_islandora.drush_batch');
  foreach ($objects as $pid) {
    $book = islandora_object_load($pid);
    $pages = array_keys(islandora_paged_content_get_pages($book));
    // $img_batch = islandora_paged_content_create_images_batch($book, $pages);
    $img_batch = bc_islandora_create_images_batch($book, $pages);
    //$pdf_batch = islandora_paged_content_create_pdf_batch($book, $pages);
    $pdf_batch = bc_islandora_create_pdf_batch($book, $pages);
    $ocr_batch = islandora_paged_content_create_ocr_batch($book, $pages, array('language' => 'eng', 'preprocess' => FALSE));
    batch_set($img_batch);
    batch_set($pdf_batch);
    batch_set($ocr_batch);
  }
  drush_backend_batch_process();
}

function drush_bc_islandora_generate_url_aliases() {
  $everything_query = <<<SPARQL
    SELECT DISTINCT ?object
    FROM <#ri>
    WHERE {
      ?object <fedora-model:hasModel> ?model
    }
SPARQL;
  $tuque = islandora_get_tuque_connection();
  $results = $tuque->repository->ri->query($everything_query, 'sparql');
  foreach ($results as $result) {
    $pid = $result['object']['value'];
    if ($object = islandora_object_load($pid)) {
      // Make the alias (if it doesn't already exist).
      $default_path = "islandora/object/{$pid}";
      if (drupal_get_path_alias($default_path) == $default_path) {
        $rels = $object->relationships->get();
        foreach ($rels as $rdf) {
          if (preg_match('/isMemberOf/', $rdf['predicate']['value']) && !in_array($rdf['object']['value'], array('islandora:root', 'islandora:collectionCModel'))) {
            drush_log(dt('Creating alias for @pid :)', array('@pid' => $object->id)), 'ok');
            islandora_pathauto_create_alias($object, 'bulkupdate');
          }
        }
      }
      else {
        drush_log(dt('Alias already exists at pid = @pid', array('@pid' => $object->id)), 'warning');
      }
    }
    else {
      drush_log(dt('Couldn\'t load object at pid = @pid', array('@pid' => $pid)), 'warning');
    }
  }
}

function drush_bc_islandora_replace_ocr($query_file) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');

  ini_set("auto_detect_line_endings", "1");

  $path_name = dirname($query_file);
  $fh = fopen($query_file, 'r');
  $line_number = 0;

  while (($data = fgetcsv($fh)) !== FALSE) {
    $line_number++;
    if ($line_number > 1) {
      $ocr_filename = str_replace('tiff', 'txt', $data[2]);
      $hocr_filename = str_replace('tiff', 'html', $data[2]);
      $type = $data[4];
      if ($type == 'page') {
        $page_pid = 'bc:bulletin-' . date('Ymd', strtotime($data[1])) . '-' . $data[3];
        $object = islandora_object_load($page_pid);
        $ocr_ds = $object['OCR'];
        $hocr_ds = $object['HOCR'];
        $ocr_ds->setContentFromFile($path_name . '/ocr/' . $ocr_filename, FALSE);
        $hocr_ds->setContentFromFile($path_name . '/ocr/' . $hocr_filename, FALSE);
        // ingest?
        // $object->ingestDatastream($datastream);
      }
    }
  }
}
