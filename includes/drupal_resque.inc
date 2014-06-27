<?php
/**
 * @file
 * Contains DrupalResque.
 */

class DrupalResque extends SystemQueue implements DrupalQueueInterface {
  /**
   * Class name that will process jobs, defaults to DrupalResqueBase.
   *
   * @var string $class_name
   */
  protected $className = 'DrupalResqueBase';

  /**
   * Setup redis server.
   *
   * @param string $name
   *   The name of the queue.
   */
  public function __construct($name) {
    parent::__construct($name);

    $redis_host = variable_get('redis_host', 'localhost');
    $redis_port = variable_get('redis_port', '6379');
    $redis_password = variable_get('redis_password', NULL);

    // Required if redis has a password, and/or on another server.
    if (!empty($redis_password)) {
      Resque::setBackend('redis://redis:' . $redis_password . '@' . $redis_host . ':' . $redis_port);
    }
    else {
      Resque::setBackend($redis_host . ':' . $redis_port);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function createItem($data) {
    $token = NULL;

    $queues = module_invoke_all('cron_queue_info');
    drupal_alter('cron_queue_info', $queues);

    // Check to see if class name was specified in the data array.
    if (!empty($queues[$this->name]['class'])) {
      $this->className = $queues[$this->name]['class'];
    }
    elseif (!empty($queues[$this->name]['worker_callback'])) {
      // Add the worker callback.
      $data['worker_callback'] = $queues[$this->name]['worker callback'];
    }
    $token = Resque::enqueue($this->name, $this->className, $data, TRUE);

    return $token;
  }

  /**
   * Set the class that is going to run the job.
   *
   * @param string $class_name
   *   The name of the class to process job.
   */
  public function setClassName($class_name) {
    $this->className = $class_name;
  }

  /**
   * Get the name of the class that is processing this job.
   */
  public function getClassName() {
    return $this->className;
  }

  /**
   * {@inheritdoc}
   */
  public function numberOfItems() {
    return Resque::size($this->name);
  }

  /**
   * Get the status of a particular job in the queue.
   *
   * @param string $token
   *   The token string returned when creating the job.
   *
   * @return int
   *   https://github.com/chrisboulton/php-resque
   *   1 = STATUS_WAITING
   *   2 = STATUS_RUNNING
   *   3 = STATUS_FAILED
   *   4 = STATUS_COMPLETE
   */
  public function getJobStatus($token) {
    $status = new Resque_Job_Status($token);
    return $status->get();
  }

  /**
   * {@inheritdoc}
   */
  public function claimItem($lease_time = 300) {
    // Not implemented.
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function releaseItem($item) {
    // Not implemented.
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function deleteItem($item) {
    // Not implemented.
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function deleteQueue() {
    // Not implemented.
    return TRUE;
  }
}