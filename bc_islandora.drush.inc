<?php

/**
 * @file
 * Custom drush commands for Barnard Digital Collections.
 *
 * Much of this was adapted from Robertson Library's drush code.
 * @see https://github.com/roblib/scripts/tree/master/drush/drupal7.
 */

require_once 'includes/bc_islandora.drush_helpers.inc';

/**
 * Implements hook_drush_command().
 */
function bc_islandora_drush_command() {
  return array(
    'bc_islandora_purge_object' => array(
      'description' => 'Purges an object from the repository.',
      'arguments' => array(
        'pids' => 'A comma-separated list of unique identifiers.',
      ),
      'aliases' => array('bcipo'),
    ),
    'bc_islandora_ingest_newspapers' => array(
      'description' => "Creates the parent issue object and uploads the pages as child objects. Requires the Drupal7 version of Islandora with tuque",
      'arguments' => array(
        'query_file' => 'The path to a CSV file that contains the information about the newspaper pages',
      ),
      'options' => array(
        'derivatives' => 'Whether to create derivatives.',
        'overwrite-existing' => 'Whether to overwrite existing objects.',
        'purge' => 'Whether to purge instead of ingesting.',
      ),
      'examples' => array(
        'drush -u 1 --uri=http://localhost ingest_newspapers /path/to/pages.csv',
      ),
      'aliases' => array('bciin'),
      // We can pass in users id on the command line using drush -u.
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    ),
    'bc_islandora_ingest_books' => array(
      'description' => 'Ingests books.',
      'arguments' => array(
        'query_file' => 'A csv file containing information about the books to be ingested.',
      ),
      'options' => array(
        'derivatives' => 'Whether to create derivatives.',
        'overwrite-existing' => 'Whether to overwrite existing objects.',
        'purge' => 'Whether to purge instead of ingesting.',
        'year' => 'Whether to import a specific year\'s book.',
      ),
      'examples' => array(
        'drush -u 1 --uri=http://localhost bciib /path/to/pages.csv',
      ),
      'aliases' => array('bciib'),
      // We can pass in users id on the command line using drush -u.
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    ),
    'bc_islandora_ingest_pubs' => array(
      'description' => 'Ingests student publications.',
      'arguments' => array(
        'directory' => 'The directory in which the publication data/files are held.',
      ),
      'options' => array(
        'derivatives' => 'Whether to create derivatives.',
        'overwrite-existing' => 'Whether to overwrite existing objects.',
        'purge' => 'Whether to purge instead of ingesting.',
        'id' => 'Whether to import the publication corresponding to a certain internal identifier.',
      ),
      'examples' => array(
        'drush -u 1 --uri=http://localhost bcii /path/to/pubs',
      ),
      'aliases' => array('bciip'),
      // We can pass in users id on the command line using drush -u.
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
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
    'bc_islandora_generate_url_aliases' => array(
      'examples' => array(
        'drush -u 1 bcigua',
      ),
      'aliases' => array('bcigua'),
      // We can pass in users id on the command line using drush -u.
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    ),
    'bc_islandora_check_ocr' => array(
      'examples' => array(
        'drush -u 1 bcico',
      ),
      'aliases' => array('bcico'),
    ),
  );
}

/**
 * Purges one or more islandora objects from the fedora repository.
 *
 * @param string $pids
 *   The pid of the object to purge, or a comma-separated list of pids.
 */
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

/**
 * Ingests newspaper objects.
 *
 * @param string $query_file
 *   A csv file containing data about the newspaper(s) to be ingested.
 */
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
    // Only parse the data if we aren't on the first line, which contains
    // column headings.
    if ($line_number > 1) {
      $type = $data[4];
      if ($type == 'issue') {
        $issue_pid = 'bc:bulletin-' . date('Ymd', strtotime($data[1]));

        // Either purge, create, or overwrite (purge then create) the object,
        // depending which options were specified on the command line.
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
          drush_bc_islandora_purge_object($page_pid);
        }
        elseif (!islandora_object_load($page_pid)) {
          try {
            create_page_object($path_name, $data, $issue_pid, $page_pid, 'newspaper');
          }
          catch (Exception $e) {
            drush_log(dt('Caught exception: @e', array('@e' => $e->getMessage())), 'warning');
          }
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

/**
 * Ingests book objects.
 *
 * @param string $query_file
 *   A csv file containing data about the newspaper(s) to be ingested.
 */
function drush_bc_islandora_ingest_books($query_file) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');

  ini_set("auto_detect_line_endings", "1");

  $fh = fopen($query_file, 'r');
  $year = drush_get_option('year');
  $books = array();

  while (($data = fgetcsv($fh)) !== FALSE) {
    $csv_year = $data[3];
    if ($year && $year != $csv_year) {
      continue;
    }
    // Some tricky stuff follows to handle books with multiple "volumes."
    $vol_parts = explode(' ', $data[1]);
    array_shift($vol_parts);
    $book_pid = 'bc:yearbook-' . implode('.', $vol_parts);
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

/**
 * Ingests student publications.
 *
 * @param string $directory
 *   The directory containing publications to ingest.
 */
function drush_bc_islandora_ingest_pubs($directory) {
  $pub_id = drush_get_option('id');
  $purge = drush_get_option('purge');

  if (is_dir($directory)) {
    // Look at each file in the supplied (argument) directory.
    foreach (scandir($directory) as $file) {
      // The full path to this file.
      $path = $directory . '/' . $file;
      // The XML data file (should follow the format: [directory name].xml).
      $xml_file = $path . '/' . $file . '.xml';
      print "\$xml_file: $xml_file\n";
      // If we didn't get a good directory or XML file, move on.
      if (!is_dir($path) || preg_match('/^\./', $file) !== 0 || !file_exists($xml_file)) {
        continue;
      }
      $pub_data = _bc_islandora_pub_data($xml_file);
      $pid_title = str_replace(' ', '-', preg_replace('/[^a-z\d\s+]/', '', strtolower($pub_data['title'])));
      $pub_pid = 'bc:' . $pid_title;
      // Publication ids that constitute "exceptions" (different behavior
      // required).
      $exceptions = array('BC12-9_1959', 'BC12-26_1970', 'BC12-30_C19711972');
      $exception = in_array(array_pop(explode('/', $path)), $exceptions);
      // If pub object doesn't exist, create it.
      if (!islandora_object_load($pub_pid) && !$exception) {
        create_pub_object($pub_pid, $pub_data);
      }
      elseif ($exception) {
        $collection_pid = variable_get('bc_islandora_pubs_pid', 'islandora:1022');
        $book_pid = create_book_object($pub_pid, _bc_islandora_pub_data($xml_file, $collection_pid));
      }
      $pub_obj = islandora_object_load($pub_pid);
      // If we got an id, only look at the corresponding dir.
      if (($pub_id && $file == $pub_id) || !$pub_id) {
        if ($exception) {
          // Yuk!
          if (!isset($book_pid)) {
            $book_pid = $pub_pid;
          }
          _bc_islandora_process_pub_pages($book_pid, $xml_file, $path, $purge);
        }
        // If non-exceptional, do the normal thing (TODO reverse this order?)
        else {
          print "Got pid: $pub_pid\n";
          // Create a book object corresponding to each pub.
          foreach (scandir($path) as $subfile) {
            $subpath = implode('/', array($path, $subfile));
            $xml_file = implode('/', array($subpath, $subfile)) . '.xml';
            if ((!is_dir($subpath) || preg_match('/^\./', $subdir) !== 0 || !file_exists($xml_file)) && !$exception) {
              continue;
            }
            $book_data = _bc_islandora_pub_data($xml_file, $pub_pid, 'book');
            $issue_pid = implode('-', array($pub_pid, $book_data['date']));
            print "Got pid: $issue_pid\n";
            if (!islandora_object_load($issue_pid)) {
              create_book_object($issue_pid, $book_data);
            }
            $issue_obj = islandora_object_load($issue_pid);
            // Create a page object for each page.
            _bc_islandora_process_pub_pages($issue_pid, $xml_file, $subpath, $purge);
          }
        }
      }
    }
  }
}

/**
 * Helper function: returns structured data about a publication.
 */
function _bc_islandora_pub_data($xml_file, $pid = '', $type = '', $page_number = '') {
  $xml = file_get_contents($xml_file);
  $mods = simplexml_load_string($xml);
  $data = array(
    'xml' => $xml,
    'collection_pid' => $pid,
    'date' => (string) $mods->originInfo->dateCreated,
    'title' => (string) $mods->titleInfo->title,
  );
  if ($type == 'page') {
    $data['genre'] = (string) $mods->genre;
    $data['title_related'] = (string) $mods->titleInfo->title;
    $data['host_id'] = (string) $mods->relatedItem[1]->identifier;
    $data['page_id'] = (string) $mods->identifier . '_' . str_pad($page_number, 3, STR_PAD_LEFT);
  }
  return $data;
}

/**
 * Helper function: processes (ingests or purges) publication pages.
 */
function _bc_islandora_process_pub_pages($parent_pid, $parent_xml_file, $path, $purge = FALSE) {
  foreach (scandir($path) as $page) {
    if (preg_match('/^\./', $page) === 0 && preg_match('/\.tif$/', $page) !== 0) {
      $page_file = $path . '/' . $page;
      $page_number_padded = str_replace('.tif', '', end(explode('_', $page)));
      $page_number = preg_replace('/^0+/', '', $page_number_padded);
      $page_pid = $parent_pid . '-' . $page_number;
      $page_data = _bc_islandora_pub_data($parent_xml_file, $issue_pid, 'page', $page_number);
      $page_data['page_number'] = $page_number;
      $page_data['title'] .= ', page ' . $page_number;
      if (!islandora_object_load($page_pid) && !$purge) {
        print "Creating page: $page_pid\n";
        var_export($page_data);
        create_page_object($page_file, $page_data, $parent_pid, $page_pid, 'pub');
      }
      elseif ($purge) {
        drush_bc_islandora_purge_object($page_pid);
      }
    }
  }
}

/**
 * Generates csv files for ingesting newspapers.
 */
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
          $lines[] = array(
            "Barnard Bulletin, " . $date_title, $date_title,
            '',
            '',
            'issue',
            $extent,
          );
        }
        $lines[] = array(
          $date_title,
          $date_title,
          $file,
          $page,
          'page',
          $extent,
        );
      }
    }
  }
  foreach ($lines as $line) {
    fputcsv($csv, $line);
  }
  fclose($csv);
}

/**
 * Helper function to set and process derivative-creation batches.
 *
 * @see includes/bc_islandora.drush_batch.inc.
 */
function _bc_islandora_create_derivatives($objects) {
  module_load_include('inc', 'bc_islandora', 'includes/bc_islandora.drush_batch');
  foreach ($objects as $pid) {
    $book = islandora_object_load($pid);
    $pages = array_keys(islandora_paged_content_get_pages($book));
    // $img_batch = islandora_paged_content_create_images_batch($book, $pages);
    $img_batch = bc_islandora_create_images_batch($book, $pages);
    // $pdf_batch = islandora_paged_content_create_pdf_batch($book, $pages);
    $pdf_batch = bc_islandora_create_pdf_batch($book, $pages);
    $ocr_batch = islandora_paged_content_create_ocr_batch($book, $pages, array('language' => 'eng', 'preprocess' => FALSE));
    batch_set($img_batch);
    batch_set($pdf_batch);
    batch_set($ocr_batch);
  }
  drush_backend_batch_process();
}

/**
 * Generates URL aliases for objects in the repo that don't have them.
 *
 * (Requires islandora_pathauto module.)
 */
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
            drush_log(dt('Creating alias for @pid', array('@pid' => $object->id)), 'ok');
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

function drush_bc_islandora_check_ocr($collection_pid = 'islandora:bookCollection') {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  // SPARQL query for isMemberOf book collection and hasModel bookCModel.
  $query = <<<EOQ
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
PREFIX fedora-rels-ext: <info:fedora/fedora-system:def/relations-external#>
SELECT ?object
FROM <#ri>
WHERE {
  ?object fedora-rels-ext:isMemberOf <info:fedora/{$collection_pid}> ;
       <fedora-model:hasModel> <info:fedora/islandora:bookCModel> ;
}
EOQ;
  $tuque = islandora_get_tuque_connection();
  $results = $tuque->repository->ri->query($query, 'sparql');
  foreach ($results as $result) {
    $pid = isset($result['object']['value']) && $result['object']['type'] == 'pid' ?
      $result['object']['value'] : NULL;
    if ($pid && $book = islandora_object_load($pid)) {
      $pages = islandora_paged_content_get_pages($book);
      foreach ($pages as $p) {
        if ($page = islandora_object_load($p['pid'])) {
          if (!isset($page['OCR'])) {
            print "Didn't get OCR at book = {$book->id}\n";
            break;
          }
          elseif (!isset($page['HOCR'])) {
            print "Didn't get HOCR at book = {$book->id}\n";
            break;
          }
        }
      }
    }
  }
}
