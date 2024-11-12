using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Enemy : MonoBehaviour
{
    public float moveSpeed = 5f;
    public float rotateSpeed = 50f;
    public float decisionInterval = 2.5f;
    public GameObject bulletPrefab;
    public Transform bulletSpawnPoint;
    public AudioClip shootSound; // �߻� ����
    public float shootVolume = 0.3f; // �߻� ������ ����

    private Transform playerTransform;
    private Rigidbody rb;
    private bool isMoving = false;
    private AudioSource audioSource; // �߻� ���� ����� ���� AudioSource

    void Start()
    {
        rb = GetComponent<Rigidbody>();
        playerTransform = GameObject.FindWithTag("Player").transform;
        StartCoroutine(DecisionLoop());

        // AudioSource ������Ʈ�� �߰��ϰ� ����
        audioSource = gameObject.AddComponent<AudioSource>();
        audioSource.volume = shootVolume;
    }

    void Update()
    {
        if (isMoving)
        {
            rb.MovePosition(transform.position + transform.forward * moveSpeed * Time.deltaTime);
        }
    }

    IEnumerator DecisionLoop()
    {
        while (true)
        {
            float decision = Random.Range(0f, 1f);

            if (decision < 0.6f)
            {
                int randomAction = Random.Range(0, 3);

                if (randomAction == 0)
                {
                    isMoving = true;
                    yield return new WaitForSeconds(2f);
                    isMoving = false;
                }
                else if (randomAction == 1)
                {
                    StartCoroutine(Rotate(-1));
                    yield return new WaitForSeconds(0.5f);
                }
                else if (randomAction == 2)
                {
                    StartCoroutine(Rotate(1));
                    yield return new WaitForSeconds(0.5f);
                }
            }
            else
            {
                Vector3 directionToPlayer = (playerTransform.position - transform.position).normalized;
                directionToPlayer.y = 0;


                Quaternion lookRotation = Quaternion.LookRotation(directionToPlayer);
                float elapsedTime = 0f;
                while (elapsedTime < 0.5f)
                {
                    transform.rotation = Quaternion.Slerp(transform.rotation, lookRotation, elapsedTime / 0.5f);
                    elapsedTime += Time.deltaTime;
                    yield return null;
                }

                if (Random.value < 0.6f)
                {
                    FireBullet();
                }

                isMoving = true;
                yield return new WaitForSeconds(2f);
                isMoving = false;
            }

            yield return new WaitForSeconds(decisionInterval - 2.5f);
        }
    }

    IEnumerator Rotate(int direction)
    {
        float elapsedTime = 0f;
        while (elapsedTime < 0.5f)
        {
            transform.Rotate(0, direction * rotateSpeed * Time.deltaTime, 0);
            elapsedTime += Time.deltaTime;
            yield return null;
        }
    }

    private void FireBullet()
    {
        Instantiate(bulletPrefab, bulletSpawnPoint.position, bulletSpawnPoint.rotation);

        // �߻� ���� ���
        if (shootSound != null && audioSource != null)
        {
            audioSource.PlayOneShot(shootSound);
        }
    }
}
